// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Controller/KeyCoreControllerBase.h"
#include "Controller/KeyCoreControllerOrder.h"
#include "Templates/UnrealTypeTraits.h"

/**
 * Compile-time solver for the Controller order (Docs/Plans/OrderOfOperations.md, O1 to O6).
 * The game module includes every Controller header and lists all Controllers once with KEYCORE_DEFINE_CONTROLLER_ORDER.
 * The solver validates the declarations and sorts them while the code compiles; a broken declaration aborts the build.
 */

/** Runtime result of the solver. Pure data, built once from the compiled order. */
struct FControllerOrder
{
	/** Controller classes in initialization order. Deinitialization runs in reverse. */
	TArray<TSubclassOf<UControllerBase>> InitOrder;

	/** Positions in InitOrder, in tick order. */
	TArray<int32> TickOrder;
};

/** Kinds of broken declarations. Each kind has one error template below with a fixed message. */
enum class EControllerOrderProblem : uint8
{
	None,
	NotAController,
	ListedTwice,
	RelationToSelf,
	RelationToUnlisted,
	InitOrderCycle,
	TickOrderCycle
};

/** First problem the solver found, with the list position of the Controller and, for relation problems, the relation position. */
struct FControllerOrderProblem
{
	EControllerOrderProblem Kind = EControllerOrderProblem::None;
	int32 ControllerIndex = INDEX_NONE;
	int32 RelationIndex = INDEX_NONE;
};

/** Constexpr result of the solver: the problem, or the init and tick order as list positions. */
template <int32 Capacity>
struct TControllerOrderResult
{
	FControllerOrderProblem Problem;
	int32 InitOrder[Capacity] = {};
	int32 TickOrder[Capacity] = {};
};

/** Always false, but only known once T is given. Lets an error template fail only when it is instantiated. */
template <typename T>
struct TControllerOrderAlwaysFalse
{
	static constexpr bool Value = false;
};

/*
 * Error templates. C++20 static_assert accepts only literal messages (plan O6), so each error kind has its own template
 * and the offending class is its template argument. The compiler prints that template name together with the message.
 */

template <typename ControllerType>
struct TControllerOrderErrorNotAController
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: a listed type does not derive from UControllerBase. The type is the template argument of TControllerOrderErrorNotAController.");
};

template <typename ControllerType>
struct TControllerOrderErrorListedTwice
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: a Controller is listed twice in KEYCORE_DEFINE_CONTROLLER_ORDER. The Controller is the template argument of TControllerOrderErrorListedTwice.");
};

template <typename ControllerType>
struct TControllerOrderErrorRelationToSelf
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: a Controller declares a relation to itself in KEYCORE_CONTROLLER_ORDER. The Controller is the template argument of TControllerOrderErrorRelationToSelf.");
};

template <typename ControllerType, typename TargetType>
struct TControllerOrderErrorRelationToUnlisted
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: a Controller declares a relation to a class that is missing from KEYCORE_DEFINE_CONTROLLER_ORDER. The template arguments of TControllerOrderErrorRelationToUnlisted are the Controller and the missing class.");
};

template <typename ControllerType>
struct TControllerOrderErrorInitOrderCycle
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: the DependsOn / RunsAfter declarations form a cycle. The template argument of TControllerOrderErrorInitOrderCycle is part of the cycle.");
};

template <typename ControllerType>
struct TControllerOrderErrorTickOrderCycle
{
	static_assert(TControllerOrderAlwaysFalse<ControllerType>::Value,
		"KeyCore controller order: the TicksAfter declarations form a cycle. The template argument of TControllerOrderErrorTickOrderCycle is part of the cycle.");
};

namespace KeyCore::Private
{
	/** Declaration of a listed type. Types that are no Controller get the empty declaration, so the solver reports them once instead of failing on a missing alias. */
	template <typename ControllerType, bool bIsController = TIsDerivedFrom<ControllerType, UControllerBase>::Value>
	struct TControllerDeclarationOf
	{
		using Type = TControllerOrderDeclaration<>;
	};

	template <typename ControllerType>
	struct TControllerDeclarationOf<ControllerType, true>
	{
		using Type = typename ControllerType::FControllerOrderDeclaration;
	};

	/** "Must come before" relation between list positions: Before[A][B] means A runs before B. */
	template <int32 Capacity>
	struct TControllerOrderGraph
	{
		bool Before[Capacity][Capacity] = {};
	};
}

/** Validates and sorts the listed Controllers at compile time. ControllerTypes is the central list (plan O1). */
template <typename... ControllerTypes>
struct TControllerOrderSolver
{
	static constexpr int32 Count = sizeof...(ControllerTypes);
	static constexpr int32 Capacity = Count > 0 ? Count : 1;

	using FResult = TControllerOrderResult<Capacity>;
	using FGraph = KeyCore::Private::TControllerOrderGraph<Capacity>;

	// Completes FResult before Solve() is used in a constant expression. Otherwise MSVC warns C4686 (return calling convention may change).
	static_assert(sizeof(FResult) > 0, "FResult must be a complete type.");

	/** Finds the first problem, or computes both orders. Usable in static_assert, so tests can check problems without breaking the build. */
	static constexpr FResult Solve()
	{
		FResult Result;
		Result.Problem = FindListProblem();
		if (Result.Problem.Kind != EControllerOrderProblem::None)
		{
			return Result;
		}

		FGraph InitGraph;
		FGraph TickGraph;
		Result.Problem = BuildGraphs(InitGraph, TickGraph);
		if (Result.Problem.Kind != EControllerOrderProblem::None)
		{
			return Result;
		}

		int32 ListOrder[Capacity] = {};
		for (int32 Index = 0; Index < Count; ++Index)
		{
			ListOrder[Index] = Index;
		}

		const int32 InitCycleIndex = SortStable(InitGraph, ListOrder, Result.InitOrder);
		if (InitCycleIndex != INDEX_NONE)
		{
			Result.Problem = { EControllerOrderProblem::InitOrderCycle, InitCycleIndex, INDEX_NONE };
			return Result;
		}

		const int32 TickCycleIndex = SortStable(TickGraph, Result.InitOrder, Result.TickOrder);
		if (TickCycleIndex != INDEX_NONE)
		{
			Result.Problem = { EControllerOrderProblem::TickOrderCycle, TickCycleIndex, INDEX_NONE };
		}
		return Result;
	}

	/** Raises the error template of the first problem, so the build stops with its message. Returns true if there is none. */
	static constexpr bool Validate()
	{
		constexpr FControllerOrderProblem Problem = Solve().Problem;
		return TProblemReport<Problem.Kind, Problem.ControllerIndex, Problem.RelationIndex>::bIsValid;
	}

	/** Turns the compiled order into runtime data. Fails the build if the declarations are broken. */
	static FControllerOrder BuildOrder()
	{
		static_assert(Validate(), "KeyCore controller order is invalid. The error above names the class.");

		constexpr FResult Result = Solve();
		const TSubclassOf<UControllerBase> ListedClasses[] = { ControllerTypes::StaticClass()..., nullptr };

		int32 InitPositionOfListIndex[Capacity] = {};
		FControllerOrder Order;
		for (int32 Position = 0; Position < Count; ++Position)
		{
			Order.InitOrder.Add(ListedClasses[Result.InitOrder[Position]]);
			InitPositionOfListIndex[Result.InitOrder[Position]] = Position;
		}
		for (int32 Position = 0; Position < Count; ++Position)
		{
			Order.TickOrder.Add(InitPositionOfListIndex[Result.TickOrder[Position]]);
		}
		return Order;
	}

private:
	template <int32 Index>
	using TControllerAt = typename TNthTypeFromParameterPack<Index, ControllerTypes...>::Type;

	template <int32 Index>
	using TDeclarationAt = typename KeyCore::Private::TControllerDeclarationOf<TControllerAt<Index>>::Type;

	/*
	 * Picks the error template of one problem kind. Only the matching specialization is instantiated, so the other kinds never
	 * touch invalid indices. (if constexpr in Validate() was tried first: MSVC still evaluated alias templates in discarded branches.)
	 */
	template <EControllerOrderProblem Kind, int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport
	{
		static constexpr bool bIsValid = true;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::NotAController, ControllerIndex, RelationIndex> : TControllerOrderErrorNotAController<TControllerAt<ControllerIndex>>
	{
		static constexpr bool bIsValid = false;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::ListedTwice, ControllerIndex, RelationIndex> : TControllerOrderErrorListedTwice<TControllerAt<ControllerIndex>>
	{
		static constexpr bool bIsValid = false;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::RelationToSelf, ControllerIndex, RelationIndex> : TControllerOrderErrorRelationToSelf<TControllerAt<ControllerIndex>>
	{
		static constexpr bool bIsValid = false;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::RelationToUnlisted, ControllerIndex, RelationIndex>
		: TControllerOrderErrorRelationToUnlisted<TControllerAt<ControllerIndex>, typename TDeclarationAt<ControllerIndex>::template TRelationAt<RelationIndex>::FTarget>
	{
		static constexpr bool bIsValid = false;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::InitOrderCycle, ControllerIndex, RelationIndex> : TControllerOrderErrorInitOrderCycle<TControllerAt<ControllerIndex>>
	{
		static constexpr bool bIsValid = false;
	};

	template <int32 ControllerIndex, int32 RelationIndex>
	struct TProblemReport<EControllerOrderProblem::TickOrderCycle, ControllerIndex, RelationIndex> : TControllerOrderErrorTickOrderCycle<TControllerAt<ControllerIndex>>
	{
		static constexpr bool bIsValid = false;
	};

	/** Checks the list itself: every entry is a Controller and listed once. */
	static constexpr FControllerOrderProblem FindListProblem()
	{
		constexpr bool IsController[] = { TIsDerivedFrom<ControllerTypes, UControllerBase>::Value..., true };
		constexpr int32 FirstIndex[] = { KeyCore::Private::FindTypeIndex<ControllerTypes, ControllerTypes...>()..., INDEX_NONE };

		for (int32 Index = 0; Index < Count; ++Index)
		{
			if (!IsController[Index])
			{
				return { EControllerOrderProblem::NotAController, Index, INDEX_NONE };
			}
		}
		for (int32 Index = 0; Index < Count; ++Index)
		{
			if (FirstIndex[Index] != Index)
			{
				return { EControllerOrderProblem::ListedTwice, Index, INDEX_NONE };
			}
		}
		return {};
	}

	/** Adds the relations of all Controllers to the graphs. Returns the first relation to itself or to an unlisted class. */
	static constexpr FControllerOrderProblem BuildGraphs(FGraph& InitGraph, FGraph& TickGraph)
	{
		FControllerOrderProblem Problem;
		int32 ControllerIndex = 0;
		(AddRelationsOf<ControllerTypes>(ControllerIndex++, InitGraph, TickGraph, Problem), ...);
		return Problem;
	}

	template <typename ControllerType>
	static constexpr void AddRelationsOf(const int32 ControllerIndex, FGraph& InitGraph, FGraph& TickGraph, FControllerOrderProblem& Problem)
	{
		using FDeclaration = typename KeyCore::Private::TControllerDeclarationOf<ControllerType>::Type;

		for (int32 RelationIndex = 0; RelationIndex < FDeclaration::RelationCount; ++RelationIndex)
		{
			const int32 TargetIndex = FDeclaration::template GetTargetIndex<ControllerTypes...>(RelationIndex);
			if (TargetIndex == ControllerIndex || TargetIndex == INDEX_NONE)
			{
				if (Problem.Kind == EControllerOrderProblem::None)
				{
					const EControllerOrderProblem Kind = TargetIndex == ControllerIndex ? EControllerOrderProblem::RelationToSelf : EControllerOrderProblem::RelationToUnlisted;
					Problem = { Kind, ControllerIndex, RelationIndex };
				}
				continue;
			}

			FGraph& Graph = FDeclaration::GetRelation(RelationIndex) == EControllerRelation::TicksAfter ? TickGraph : InitGraph;
			Graph.Before[TargetIndex][ControllerIndex] = true;
		}
	}

	/**
	 * Stable topological sort: at every position, the first node in Priority whose predecessors are all placed is taken.
	 * Returns INDEX_NONE on success, otherwise a node that lies on a cycle.
	 */
	static constexpr int32 SortStable(const FGraph& Graph, const int32 (&Priority)[Capacity], int32 (&OutOrder)[Capacity])
	{
		bool Placed[Capacity] = {};
		for (int32 Position = 0; Position < Count; ++Position)
		{
			const int32 Chosen = FindFirstReadyNode(Graph, Priority, Placed);
			if (Chosen == INDEX_NONE)
			{
				return FindNodeOnCycle(Graph, Placed);
			}
			Placed[Chosen] = true;
			OutOrder[Position] = Chosen;
		}
		return INDEX_NONE;
	}

	static constexpr int32 FindFirstReadyNode(const FGraph& Graph, const int32 (&Priority)[Capacity], const bool (&Placed)[Capacity])
	{
		for (int32 PriorityIndex = 0; PriorityIndex < Count; ++PriorityIndex)
		{
			const int32 Node = Priority[PriorityIndex];
			if (!Placed[Node] && FindUnplacedPredecessor(Graph, Placed, Node) == INDEX_NONE)
			{
				return Node;
			}
		}
		return INDEX_NONE;
	}

	static constexpr int32 FindUnplacedPredecessor(const FGraph& Graph, const bool (&Placed)[Capacity], const int32 Node)
	{
		for (int32 Candidate = 0; Candidate < Count; ++Candidate)
		{
			if (Graph.Before[Candidate][Node] && !Placed[Candidate])
			{
				return Candidate;
			}
		}
		return INDEX_NONE;
	}

	/**
	 * Every unplaced node has an unplaced predecessor when the sort is stuck. Walking predecessors Count times from any
	 * unplaced node therefore ends on a cycle, so the reported class is part of the cycle, not only behind it.
	 */
	static constexpr int32 FindNodeOnCycle(const FGraph& Graph, const bool (&Placed)[Capacity])
	{
		int32 Node = INDEX_NONE;
		for (int32 Candidate = 0; Candidate < Count && Node == INDEX_NONE; ++Candidate)
		{
			if (!Placed[Candidate])
			{
				Node = Candidate;
			}
		}
		for (int32 Step = 0; Step < Count; ++Step)
		{
			Node = FindUnplacedPredecessor(Graph, Placed, Node);
		}
		return Node;
	}
};

/**
 * Defines FunctionName, which returns the compiled Controller order of all listed Controllers (plan O1). Used once, in the game module.
 * Example: KEYCORE_DEFINE_CONTROLLER_ORDER(GetSimpleLifeSimControllerOrder, UTimeController, UWeatherController)
 */
#define KEYCORE_DEFINE_CONTROLLER_ORDER(FunctionName, ...) \
	const FControllerOrder& FunctionName() \
	{ \
		static const FControllerOrder Order = TControllerOrderSolver<__VA_ARGS__>::BuildOrder(); \
		return Order; \
	}
