// Copyright Torben Borghoff, Keygen-Games

#include "Controller/KeyCoreControllerOrderSolver.h"

/*
 * Compile-time tests of the Controller order solver (Docs/Plans/OrderOfOperations.md, O8). They run in every build:
 * a failing check stops the build. Broken graphs are checked through Solve(), which reports the problem without raising
 * the error templates, so these tests never break the build on purpose.
 * The test Controllers are plain C++ classes without UCLASS: they are only used as types and never constructed.
 */

namespace KeyCoreControllerOrderStaticTests
{
	class FControllerA : public UControllerBase
	{
	};

	class FControllerB : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TDependsOn<FControllerA>)
	};

	class FControllerC : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<FControllerB>)
	};

	class FTicksAfterC : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<FControllerC>)
	};

	class FSelfRelated : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<FSelfRelated>)
	};

	class FCycleFirst;

	class FCycleSecond : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<FCycleFirst>)
	};

	class FCycleFirst : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<FCycleSecond>)
	};

	/** Depends on the cycle without being part of it, so the solver must not report it. */
	class FBehindCycle : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TRunsAfter<FCycleFirst>)
	};

	class FTickCycleSecond;

	class FTickCycleFirst : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<FTickCycleSecond>)
	};

	class FTickCycleSecond : public UControllerBase
	{
	public:
		KEYCORE_CONTROLLER_ORDER(TTicksAfter<FTickCycleFirst>)
	};

	/** Has no relations, so it only moves when others must go first. */
	class FUnrelated : public UControllerBase
	{
	};

	struct FNotAController
	{
	};

	template <typename SolverType, int32 ExpectedCount>
	constexpr bool HasInitOrder(const int32 (&Expected)[ExpectedCount])
	{
		constexpr typename SolverType::FResult Result = SolverType::Solve();
		static_assert(ExpectedCount == SolverType::Count, "The expected order must list every Controller.");
		if (Result.Problem.Kind != EControllerOrderProblem::None)
		{
			return false;
		}
		for (int32 Position = 0; Position < ExpectedCount; ++Position)
		{
			if (Result.InitOrder[Position] != Expected[Position])
			{
				return false;
			}
		}
		return true;
	}

	template <typename SolverType, int32 ExpectedCount>
	constexpr bool HasTickOrder(const int32 (&Expected)[ExpectedCount])
	{
		constexpr typename SolverType::FResult Result = SolverType::Solve();
		static_assert(ExpectedCount == SolverType::Count, "The expected order must list every Controller.");
		if (Result.Problem.Kind != EControllerOrderProblem::None)
		{
			return false;
		}
		for (int32 Position = 0; Position < ExpectedCount; ++Position)
		{
			if (Result.TickOrder[Position] != Expected[Position])
			{
				return false;
			}
		}
		return true;
	}

	template <typename SolverType>
	constexpr bool HasProblem(const EControllerOrderProblem Kind, const int32 ControllerIndex, const int32 RelationIndex)
	{
		constexpr FControllerOrderProblem Problem = SolverType::Solve().Problem;
		return Problem.Kind == Kind && Problem.ControllerIndex == ControllerIndex && Problem.RelationIndex == RelationIndex;
	}

	// Valid lists: init order.
	static_assert(HasProblem<TControllerOrderSolver<>>(EControllerOrderProblem::None, INDEX_NONE, INDEX_NONE), "An empty list is valid.");
	static_assert(HasInitOrder<TControllerOrderSolver<FControllerA, FControllerB>>({ 0, 1 }), "Without conflicts, the list order is kept.");
	static_assert(HasInitOrder<TControllerOrderSolver<FControllerB, FControllerA>>({ 1, 0 }), "DependsOn moves the dependency before its dependent.");
	static_assert(HasInitOrder<TControllerOrderSolver<FControllerC, FControllerA, FControllerB>>({ 1, 2, 0 }), "RunsAfter and DependsOn chain: A, B, C.");
	static_assert(HasInitOrder<TControllerOrderSolver<FControllerB, FUnrelated, FControllerA>>({ 1, 2, 0 }), "The sort is stable: an unrelated Controller keeps its place before the moved ones.");

	// Valid lists: tick order.
	static_assert(HasTickOrder<TControllerOrderSolver<FControllerB, FControllerA>>({ 1, 0 }), "Without TicksAfter, the tick order is the init order.");
	static_assert(HasTickOrder<TControllerOrderSolver<FTicksAfterC, FControllerA, FControllerB, FControllerC>>({ 1, 2, 3, 0 }), "TicksAfter moves the Controller behind its tick target.");
	static_assert(HasInitOrder<TControllerOrderSolver<FTicksAfterC, FControllerA, FControllerB, FControllerC>>({ 0, 1, 2, 3 }), "TicksAfter does not change the init order.");

	// Broken lists: each problem kind, with the reported Controller and relation.
	static_assert(HasProblem<TControllerOrderSolver<FControllerA, FNotAController>>(EControllerOrderProblem::NotAController, 1, INDEX_NONE), "A type that is no Controller is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FControllerA, FControllerA>>(EControllerOrderProblem::ListedTwice, 1, INDEX_NONE), "The second entry of a duplicate is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FSelfRelated>>(EControllerOrderProblem::RelationToSelf, 0, 0), "A relation to itself is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FControllerB>>(EControllerOrderProblem::RelationToUnlisted, 0, 0), "A relation to an unlisted class is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FCycleFirst, FCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 1, INDEX_NONE)
		|| HasProblem<TControllerOrderSolver<FCycleFirst, FCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 0, INDEX_NONE), "An init cycle is reported.");
	static_assert(!HasProblem<TControllerOrderSolver<FBehindCycle, FCycleFirst, FCycleSecond>>(EControllerOrderProblem::InitOrderCycle, 0, INDEX_NONE)
		&& TControllerOrderSolver<FBehindCycle, FCycleFirst, FCycleSecond>::Solve().Problem.Kind == EControllerOrderProblem::InitOrderCycle, "The reported class lies on the cycle, not behind it.");
	static_assert(TControllerOrderSolver<FTickCycleFirst, FTickCycleSecond>::Solve().Problem.Kind == EControllerOrderProblem::TickOrderCycle, "A tick cycle is reported.");
	static_assert(HasProblem<TControllerOrderSolver<FNotAController, FControllerA, FControllerA>>(EControllerOrderProblem::NotAController, 0, INDEX_NONE), "List problems are reported before relation problems.");
}
