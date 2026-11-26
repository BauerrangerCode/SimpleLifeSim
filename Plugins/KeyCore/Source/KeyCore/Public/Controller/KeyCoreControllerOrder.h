// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Containers/Array.h"
#include "Templates/SubclassOf.h"
#include "Templates/UnrealTypeTraits.h"

#include <type_traits>

class UControllerBase;

/**
 * Declarations a Controller makes about its order relative to other Controllers (Docs/Plans/OrderOfOperations.md, O3).
 * This header is light, so every Controller header can include it. The compile-time solver lives in KeyCoreControllerOrderSolver.h.
 */

/** Kind of relation from one Controller to another. */
enum class EControllerRelation : uint8
{
	/** Needs access to the target: the target is initialized before and delivered through ReceiveDependency. */
	DependsOn,
	/** Order only: initialized after the target, without access to it. */
	RunsAfter,
	/** Tick order only: ticked after the target. */
	TicksAfter
};

/** Common base of the relation tags. Holds the target type and the relation kind, so the solver reads both without knowing the tag. */
template <typename TargetType, EControllerRelation InRelation>
struct TControllerRelation
{
	using FTarget = TargetType;
	static constexpr EControllerRelation Relation = InRelation;
};

/** The declaring Controller needs access to TargetType. */
template <typename TargetType>
struct TDependsOn : TControllerRelation<TargetType, EControllerRelation::DependsOn>
{
};

/** The declaring Controller is initialized after TargetType, without access to it. */
template <typename TargetType>
struct TRunsAfter : TControllerRelation<TargetType, EControllerRelation::RunsAfter>
{
};

/** The declaring Controller is ticked after TargetType. */
template <typename TargetType>
struct TTicksAfter : TControllerRelation<TargetType, EControllerRelation::TicksAfter>
{
};

namespace KeyCore::Private
{
	/** Position of SearchedType in Types, or INDEX_NONE. The solver uses it to turn relation targets into list positions. */
	template <typename SearchedType, typename... Types>
	constexpr int32 FindTypeIndex()
	{
		constexpr bool Matches[] = { std::is_same_v<SearchedType, Types>..., false };
		for (int32 Index = 0; Index < static_cast<int32>(sizeof...(Types)); ++Index)
		{
			if (Matches[Index])
			{
				return Index;
			}
		}
		return INDEX_NONE;
	}
}

/** All relations one Controller declares. Created by KEYCORE_CONTROLLER_ORDER; UControllerBase declares the empty one. */
template <typename... RelationTypes>
struct TControllerOrderDeclaration
{
	static constexpr int32 RelationCount = sizeof...(RelationTypes);

	/** Relation tag at RelationIndex. The solver uses it to name the target of a broken relation in a compiler error. */
	template <int32 RelationIndex>
	using TRelationAt = typename TNthTypeFromParameterPack<RelationIndex, RelationTypes...>::Type;

	/** Kind of the relation at RelationIndex. */
	static constexpr EControllerRelation GetRelation(const int32 RelationIndex)
	{
		constexpr EControllerRelation Relations[] = { RelationTypes::Relation..., EControllerRelation::DependsOn };
		return Relations[RelationIndex];
	}

	/** Position of the target of the relation at RelationIndex in ControllerTypes, or INDEX_NONE if the target is not listed. */
	template <typename... ControllerTypes>
	static constexpr int32 GetTargetIndex(const int32 RelationIndex)
	{
		constexpr int32 TargetIndices[] = { KeyCore::Private::FindTypeIndex<typename RelationTypes::FTarget, ControllerTypes...>()..., INDEX_NONE };
		return TargetIndices[RelationIndex];
	}

	/** Classes of all DependsOn targets. Generated from the declaration, so the delivered dependencies cannot drift from it. */
	static TArray<TSubclassOf<UControllerBase>> GetDependencyClasses()
	{
		TArray<TSubclassOf<UControllerBase>> DependencyClasses;
		(AddDependencyClass<RelationTypes>(DependencyClasses), ...);
		return DependencyClasses;
	}

private:
	template <typename RelationType>
	static void AddDependencyClass(TArray<TSubclassOf<UControllerBase>>& DependencyClasses)
	{
		if constexpr (RelationType::Relation == EControllerRelation::DependsOn)
		{
			DependencyClasses.Add(RelationType::FTarget::StaticClass());
		}
	}
};

/**
 * Declares the order relations of a Controller (Docs/Plans/OrderOfOperations.md, O3). Place it in the public section of the class body.
 * Example: KEYCORE_CONTROLLER_ORDER(TDependsOn<UTimeController>, TTicksAfter<UTimeController>)
 * It also implements GetDependencies(), which must never be overridden by hand.
 */
#define KEYCORE_CONTROLLER_ORDER(...) \
	using FControllerOrderDeclaration = TControllerOrderDeclaration<__VA_ARGS__>; \
	virtual TArray<TSubclassOf<UControllerBase>> GetDependencies() const override \
	{ \
		return FControllerOrderDeclaration::GetDependencyClasses(); \
	}
