// Copyright Torben Borghoff, Keygen-Games

#include "Controller/KeyCoreControllerOrderSolver.h"

/*
 * Compile-time tests of the Controller order solver (Docs/Plans/OrderOfOperations.md, O8). They run in every build:
 * a failing check stops the build. Broken graphs are checked through Solve(), which reports the problem without raising
 * the error templates, so these tests never break the build on purpose.
 * The test Controllers are plain C++ classes without UCLASS: they are only used as types and never constructed.
 * They derive from UControllerBase, so they carry the U prefix of the Epic standard even without reflection.
 */

namespace KeyCoreControllerOrderStaticTests
{
	class UControllerA : public UControllerBase
	{
	};

	class UControllerB : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TDependsOn<UControllerA>)
	};

	class UControllerC : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<UControllerB>)
	};

	class UTicksAfterC : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<UControllerC>)
	};

	class USelfRelated : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<USelfRelated>)
	};

	class UCycleFirst;

	class UCycleSecond : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<UCycleFirst>)
	};

	class UCycleFirst : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<UCycleSecond>)
	};

	/** Depends on the cycle without being part of it, so the solver must not report it. */
	class UBehindCycle : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<UCycleFirst>)
	};

	class UTickCycleSecond;

	class UTickCycleFirst : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<UTickCycleSecond>)
	};

	class UTickCycleSecond : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<UTickCycleFirst>)
	};

	/** Has no relations, so it only moves when others must go first. */
	class UUnrelated : public UControllerBase
	{
	};

	struct FNotAController
	{
	};

	/** True if the list is valid and the order selected by Order matches Expected. Order picks InitOrder or TickOrder of the result. */
	template <typename SolverType, int32 (SolverType::FResult::*Order)[SolverType::Capacity], int32 ExpectedCount>
	constexpr bool HasOrder(const int32 (&Expected)[ExpectedCount])
	{
		constexpr typename SolverType::FResult Result = SolverType::Solve();
		static_assert(ExpectedCount == SolverType::Count, "The expected order must list every Controller.");
		if (Result.Problem.Kind != EControllerOrderProblem::None)
		{
			return false;
		}
		for (int32 Position = 0; Position < ExpectedCount; ++Position)
		{
			if ((Result.*Order)[Position] != Expected[Position])
			{
				return false;
			}
		}
		return true;
	}

	template <typename SolverType, int32 ExpectedCount>
	constexpr bool HasInitOrder(const int32 (&Expected)[ExpectedCount])
	{
		return HasOrder<SolverType, &SolverType::FResult::InitOrder>(Expected);
	}

	template <typename SolverType, int32 ExpectedCount>
	constexpr bool HasTickOrder(const int32 (&Expected)[ExpectedCount])
	{
		return HasOrder<SolverType, &SolverType::FResult::TickOrder>(Expected);
	}

	template <typename SolverType>
	constexpr bool HasProblem(const EControllerOrderProblem Kind, const int32 ControllerIndex, const int32 RelationIndex)
	{
		constexpr FControllerOrderProblem Problem = SolverType::Solve().Problem;
		return Problem.Kind == Kind && Problem.ControllerIndex == ControllerIndex && Problem.RelationIndex == RelationIndex;
	}

	// Valid lists: init order.
	static_assert(HasProblem<TControllerOrderSolver<>>(EControllerOrderProblem::None, INDEX_NONE, INDEX_NONE), "An empty list is valid.");
	static_assert(HasInitOrder<TControllerOrderSolver<UControllerA, UControllerB>>({ 0, 1 }), "Without conflicts, the list order is kept.");
	static_assert(HasInitOrder<TControllerOrderSolver<UControllerB, UControllerA>>({ 1, 0 }), "DependsOn moves the dependency before its dependent.");
	static_assert(HasInitOrder<TControllerOrderSolver<UControllerC, UControllerA, UControllerB>>({ 1, 2, 0 }), "RunsAfter and DependsOn chain: A, B, C.");
	static_assert(HasInitOrder<TControllerOrderSolver<UControllerB, UUnrelated, UControllerA>>({ 1, 2, 0 }), "The sort is stable: an unrelated Controller keeps its place before the moved ones.");

	// Valid lists: tick order.
	static_assert(HasTickOrder<TControllerOrderSolver<UControllerB, UControllerA>>({ 1, 0 }), "Without TicksAfter, the tick order is the init order.");
	static_assert(HasTickOrder<TControllerOrderSolver<UTicksAfterC, UControllerA, UControllerB, UControllerC>>({ 1, 2, 3, 0 }), "TicksAfter moves the Controller behind its tick target.");
	static_assert(HasInitOrder<TControllerOrderSolver<UTicksAfterC, UControllerA, UControllerB, UControllerC>>({ 0, 1, 2, 3 }), "TicksAfter does not change the init order.");

	// Broken lists: each problem kind, with the reported Controller and relation.
	static_assert(HasProblem<TControllerOrderSolver<UControllerA, FNotAController>>(EControllerOrderProblem::NotAController, 1, INDEX_NONE), "A type that is no Controller is reported.");
	static_assert(HasProblem<TControllerOrderSolver<UControllerA, UControllerA>>(EControllerOrderProblem::ListedTwice, 1, INDEX_NONE), "The second entry of a duplicate is reported.");
	static_assert(HasProblem<TControllerOrderSolver<USelfRelated>>(EControllerOrderProblem::RelationToSelf, 0, 0), "A relation to itself is reported.");
	static_assert(HasProblem<TControllerOrderSolver<UControllerB>>(EControllerOrderProblem::RelationToUnlisted, 0, 0), "A relation to an unlisted class is reported.");
	static_assert(HasProblem<TControllerOrderSolver<UCycleFirst, UCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 1, INDEX_NONE)
		|| HasProblem<TControllerOrderSolver<UCycleFirst, UCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 0, INDEX_NONE), "An init cycle is reported.");
	static_assert(!HasProblem<TControllerOrderSolver<UBehindCycle, UCycleFirst, UCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 0, INDEX_NONE)
		&& TControllerOrderSolver<UBehindCycle, UCycleFirst, UCycleSecond>::Solve().Problem.Kind == EControllerOrderProblem::InitOrderCycle, "The reported class lies on the cycle, not behind it.");
	static_assert(TControllerOrderSolver<UTickCycleFirst, UTickCycleSecond>::Solve().Problem.Kind == EControllerOrderProblem::TickOrderCycle, "A tick cycle is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FNotAController, UControllerA, UControllerA>>(EControllerOrderProblem::NotAController, 0, INDEX_NONE), "List problems are reported before relation problems.");
}
