/***********************************************************************
PointTwoNTree - Abstract class for n-dimensional spatial trees
(quadtrees, octrees, etc.) with dynamic point insertion/removal.
Copyright (c) 2004-2005 Oliver Kreylos

This file is part of the Templatized Geometry Library (TGL).

The Templatized Geometry Library is free software; you can redistribute
it and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The Templatized Geometry Library is distributed in the hope that it will
be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with the Templatized Geometry Library; if not, write to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
***********************************************************************/

#define GEOMETRY_POINTTWONTREE_IMPLEMENTATION

#include <Math/Constants.h>

#include <Geometry/PointTwoNTree.h>

namespace Geometry {

/**************************************************
Methods of class PointTwoNTree::PointNotFoundError:
**************************************************/

template <class StoredPointParam>
inline
PointTwoNTree<StoredPointParam>::PointNotFoundError::PointNotFoundError(
	const typename PointTwoNTree<StoredPointParam>::StoredPoint& sPoint)
	:std::runtime_error("Point not found in PointTwoNTree"),
	 point(sPoint)
	{
	}

/******************************************************
Methods of class PointTwoNTree::NoClosePointFoundError:
******************************************************/

template <class StoredPointParam>
inline
PointTwoNTree<StoredPointParam>::NoClosePointFoundError::NoClosePointFoundError(
	const typename PointTwoNTree<StoredPointParam>::Point& sQueryPoint)
	:std::runtime_error("No close point found in PointTwoNTree"),
	 queryPoint(sQueryPoint)
	{
	}

/************************************
Methods of class PointTwoNTree::Node:
************************************/

template <class StoredPointParam>
inline
const typename PointTwoNTree<StoredPointParam>::StoredPointListItem*
PointTwoNTree<StoredPointParam>::Node::findClosePoint(
	const typename PointTwoNTree<StoredPointParam>::Point& nodeCenter,
	typename PointTwoNTree<StoredPointParam>::Scalar nodeSize,
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint) const
	{
	if(leaf)
		{
		/* Return first point in this node (or null pointer if there are none): */
		return firstItem;
		}
	else
		{
		/* Calculate the child traversal order: */
		int traversalOrderMask=0x0;
		for(int j=0;j<dimension;++j)
			if(queryPoint[j]>=nodeCenter[j])
				traversalOrderMask|=(1<<j);
		
		/* Traverse the children: */
		Scalar childNodeSize=Math::div2(nodeSize);
		const StoredPointListItem* result=0;
		for(int child=0;child<numChildren&&result==0;++child)
			{
			int childIndex=child^traversalOrderMask;
			Point childNodeCenter=nodeCenter;
			for(int i=0;i<dimension;++i)
				if(childIndex&(1<<i))
					childNodeCenter[i]+=childNodeSize;
				else
					childNodeCenter[i]-=childNodeSize;
			
			result=children->children[childIndex].findClosePoint(childNodeCenter,childNodeSize,queryPoint);
			}
		
		return result;
		}
	}

template <class StoredPointParam>
inline
const typename PointTwoNTree<StoredPointParam>::StoredPointListItem*
PointTwoNTree<StoredPointParam>::Node::findClosestPoint(
	const typename PointTwoNTree<StoredPointParam>::Point& nodeCenter,
	typename PointTwoNTree<StoredPointParam>::Scalar nodeSize,
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint,
	typename PointTwoNTree<StoredPointParam>::Scalar& minDist2) const
	{
	if(leaf)
		{
		/* Find the closest point in this node (or null pointer if the node is empty): */
		const StoredPointListItem* result=0;
		for(const StoredPointListItem* iPtr=firstItem;iPtr!=0;iPtr=iPtr->succ)
			{
			Scalar dist2=sqrDist(queryPoint,iPtr->point);
			if(minDist2>dist2)
				{
				minDist2=dist2;
				result=iPtr;
				}
			}
		
		return result;
		}
	else
		{
		/* Calculate the child traversal order: */
		int traversalOrderMask=0x0;
		for(int i=0;i<dimension;++i)
			if(queryPoint[i]>=nodeCenter[i])
				traversalOrderMask|=(1<<i);
		
		/* Traverse the children: */
		Scalar childNodeSize=Math::div2(nodeSize);
		const StoredPointListItem* result=0;
		for(int child=0;child<numChildren;++child)
			{
			int childIndex=child^traversalOrderMask;
			
			/* Cull the child if it is too far away from the query point: */
			bool mustTraverse=true;
			for(int i=0;i<dimension;++i)
				if((child&(1<<i))!=0&&Math::sqr(nodeCenter[i]-queryPoint[i])>=minDist2)
					{
					mustTraverse=false;
					break;
					}
			if(mustTraverse)
				{
				Point childNodeCenter=nodeCenter;
				for(int i=0;i<dimension;++i)
					if(childIndex&(1<<i))
						childNodeCenter[i]+=childNodeSize;
					else
						childNodeCenter[i]-=childNodeSize;
				
				const StoredPointListItem* childResult=children->children[childIndex].findClosestPoint(childNodeCenter,childNodeSize,queryPoint,minDist2);
				if(childResult!=0)
					result=childResult;
				}
			}
		
		return result;
		}
	}

template <class StoredPointParam>
inline
void
PointTwoNTree<StoredPointParam>::Node::findClosestPoints(
	const typename PointTwoNTree<StoredPointParam>::Point& nodeCenter,
	typename PointTwoNTree<StoredPointParam>::Scalar nodeSize,
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint,
	typename PointTwoNTree<StoredPointParam>::ClosePointSet& closestPoints) const
	{
	if(leaf)
		{
		/* Process all points in this node: */
		for(const StoredPointListItem* iPtr=firstItem;iPtr!=0;iPtr=iPtr->succ)
			{
			Scalar dist2=sqrDist(queryPoint,iPtr->point);
			closestPoints.insertPoint(iPtr->point,dist2);
			}
		}
	else
		{
		/* Calculate the child traversal order: */
		int traversalOrderMask=0x0;
		for(int i=0;i<dimension;++i)
			if(queryPoint[i]>=nodeCenter[i])
				traversalOrderMask|=(1<<i);
		
		/* Traverse the children: */
		Scalar childNodeSize=Math::div2(nodeSize);
		for(int child=0;child<numChildren;++child)
			{
			int childIndex=child^traversalOrderMask;
			
			/* Cull the child if it is too far away from the query point: */
			bool mustTraverse=true;
			for(int i=0;i<dimension;++i)
				if((child&(1<<i))!=0&&Math::sqr(nodeCenter[i]-queryPoint[i])>=closestPoints.getMaxSqrDist())
					{
					mustTraverse=false;
					break;
					}
			if(mustTraverse)
				{
				Point childNodeCenter=nodeCenter;
				for(int i=0;i<dimension;++i)
					if(childIndex&(1<<i))
						childNodeCenter[i]+=childNodeSize;
					else
						childNodeCenter[i]-=childNodeSize;
				
				children->children[childIndex].findClosestPoints(childNodeCenter,childNodeSize,queryPoint,closestPoints);
				}
			}
		}
	}

/******************************
Methods of class PointTwoNTree:
******************************/

template <class StoredPointParam>
inline
void
PointTwoNTree<StoredPointParam>::deleteSubtree(
	typename PointTwoNTree<StoredPointParam>::Node* node)
	{
	if(!node->leaf)
		{
		for(int i=0;i<Node::numChildren;++i)
			deleteSubtree(&(node->children->children[i]));
		node->children->~NodeBlock();
		nodeBlockAllocator.free(node->children);
		}
	else
		{
		while(node->firstItem!=0)
			{
			StoredPointListItem* succ=node->firstItem->succ;
			node->firstItem->~StoredPointListItem();
			listItemAllocator.free(node->firstItem);
			node->firstItem=succ;
			}
		}
	}

template <class StoredPointParam>
inline
PointTwoNTree<StoredPointParam>::PointTwoNTree(
	const typename PointTwoNTree<StoredPointParam>::Point& sRootCenter,
	typename PointTwoNTree<StoredPointParam>::Scalar sRootSize)
	:rootCenter(sRootCenter),rootSize(sRootSize),
	 splitThreshold(12),mergeThreshold(8)
	{
	}

template <class StoredPointParam>
inline
void
PointTwoNTree<StoredPointParam>::setSplitThreshold(
	int newSplitThreshold)
	{
	splitThreshold=newSplitThreshold;
	}

template <class StoredPointParam>
inline
void
PointTwoNTree<StoredPointParam>::setMergeThreshold(
	int newMergeThreshold)
	{
	mergeThreshold=newMergeThreshold;
	}

template <class StoredPointParam>
inline
const typename PointTwoNTree<StoredPointParam>::StoredPoint&
PointTwoNTree<StoredPointParam>::insertPoint(
	const typename PointTwoNTree<StoredPointParam>::StoredPoint& newPoint)
	{
	while(true)
		{
		/* Check if the root node contains the new point: */
		bool rootContainsPoint=true;
		for(int i=0;i<dimension;++i)
			if(newPoint[i]<rootCenter[i]-rootSize||newPoint[i]>=rootCenter[i]+rootSize)
				{
				rootContainsPoint=false;
				break;
				}
		
		if(rootContainsPoint)
			break;
		
		/* Create a new root node above the current root to enlarge the tree's domain: */
		int childIndex=0x0;
		for(int j=0;j<dimension;++j)
			if(newPoint[j]<rootCenter[j])
				{
				childIndex|=(1<<j);
				rootCenter[j]-=rootSize;
				}
			else
				rootCenter[j]+=rootSize;
		rootSize+=rootSize;
		NodeBlock* children=new(nodeBlockAllocator.allocate()) NodeBlock;
		children->children[childIndex]=root;
		
		/* Make root an internal node: */
		root.numListItems=0;
		root.firstItem=0;
		root.leaf=false;
		root.children=children;
		}
	
	/* Traverse the tree until a leaf node containing the new point's position is found: */
	Point nodeCenter=rootCenter;
	Scalar nodeSize=rootSize;
	Node* node=&root;
	while(!node->leaf)
		{
		/* Find the index of the child node containing the given point: */
		nodeSize=Math::div2(nodeSize);
		int childNodeIndex=0x0;
		for(int j=0;j<dimension;++j)
			{
			if(newPoint[j]>=nodeCenter[j])
				{
				nodeCenter[j]+=nodeSize;
				childNodeIndex|=(1<<j);
				}
			else
				nodeCenter[j]-=nodeSize;
			}
		node=&(node->children->children[childNodeIndex]);
		}
	
	/* Add a new item to the found leaf node's stored point list: */
	StoredPointListItem* newItem=new(listItemAllocator.allocate()) StoredPointListItem(newPoint);
	newItem->succ=node->firstItem;
	node->firstItem=newItem;
	++node->numListItems;
	
	if(node->numListItems>splitThreshold)
		{
		/* Split the node: */
		NodeBlock* children=new(nodeBlockAllocator.allocate()) NodeBlock;
		
		/* Distribute the node's list items amongst its children: */
		StoredPointListItem* liPtr=node->firstItem;
		while(liPtr!=0)
			{
			StoredPointListItem* succ=liPtr->succ;
			
			/* Find the child node containing the list item: */
			int childIndex=0x0;
			for(int j=0;j<dimension;++j)
				if(liPtr->point[j]>=nodeCenter[j])
					childIndex|=(1<<j);
			
			/* Insert the list item into the child node's list: */
			liPtr->succ=children->children[childIndex].firstItem;
			children->children[childIndex].firstItem=liPtr;
			++children->children[childIndex].numListItems;
			
			liPtr=succ;
			}
		
		/* Make the node an internal node: */
		node->numListItems=0;
		node->firstItem=0;
		node->leaf=false;
		node->children=children;
		}
	
	return newItem->point;
	}

template <class StoredPointParam>
inline
void
PointTwoNTree<StoredPointParam>::removePoint(
	const typename PointTwoNTree<StoredPointParam>::StoredPoint& removePoint)
	{
	/* Traverse the tree until a leaf node containing the given point's position is found: */
	Point nodeCenter=rootCenter;
	Scalar nodeSize=rootSize;
	Node* parent=0;
	Node* node=&root;
	while(!node->leaf)
		{
		/* Find the index of the child node containing the given point: */
		nodeSize=Math::div2(nodeSize);
		int childNodeIndex=0x0;
		for(int j=0;j<dimension;++j)
			{
			if(removePoint[j]>=nodeCenter[j])
				{
				nodeCenter[j]+=nodeSize;
				childNodeIndex|=(1<<j);
				}
			else
				nodeCenter[j]-=nodeSize;
			}
		parent=node;
		node=&(node->children->children[childNodeIndex]);
		}
	
	/* Find the first identical point in the node's stored point list: */
	StoredPointListItem* ptr1=0;
	StoredPointListItem* ptr2;
	for(ptr2=node->firstItem;ptr2!=0&&ptr2->point!=removePoint;ptr1=ptr2,ptr2=ptr2->succ)
		;
	if(ptr2==0)
		throw PointNotFoundError(removePoint);

	/* Remove the point (if found): */
	if(ptr1!=0)
		ptr1->succ=ptr2->succ;
	else
		node->firstItem=ptr2->succ;
	ptr2->~StoredPointListItem();
	listItemAllocator.free(ptr2);
	--node->numListItems;
	
	if(node->numListItems<mergeThreshold&&parent!=0)
		{
		/* Check if the node's parent can be merged: */
		unsigned short numParentListItems=0;
		for(int i=0;i<Node::numChildren&&numParentListItems<mergeThreshold;++i)
			{
			if(parent->children->children[i].leaf)
				numParentListItems+=parent->children->children[i].numListItems;
			else
				numParentListItems=mergeThreshold; // Easy way out; can't and won't merge
			}
		
		if(numParentListItems<mergeThreshold)
			{
			/* Merge the parent node's children: */
			StoredPointListItem* parentFirstItem=0;
			for(int i=0;i<Node::numChildren;++i)
				{
				Node* child=&(parent->children->children[i]);
				StoredPointListItem* liPtr=child->firstItem;
				while(liPtr!=0)
					{
					StoredPointListItem* succ=liPtr->succ;
					
					liPtr->succ=parentFirstItem;
					parentFirstItem=liPtr;
					
					liPtr=succ;
					}
				child->numListItems=0;
				child->firstItem=0;
				}
			
			/* Make the parent node a leaf node: */
			parent->children->~NodeBlock();
			nodeBlockAllocator.free(parent->children);
			parent->children=0;
			parent->leaf=true;
			parent->numListItems=numParentListItems;
			parent->firstItem=parentFirstItem;
			}
		}
	}

template <class StoredPointParam>
inline
const typename PointTwoNTree<StoredPointParam>::StoredPoint&
PointTwoNTree<StoredPointParam>::findClosePoint(
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint) const
	{
	const StoredPointListItem* result=root.findClosePoint(rootCenter,rootSize,queryPoint);
	if(result==0)
		throw NoClosePointFoundError(queryPoint);
	
	return result->point;
	}

template <class StoredPointParam>
inline
const typename PointTwoNTree<StoredPointParam>::StoredPoint&
PointTwoNTree<StoredPointParam>::findClosestPoint(
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint) const
	{
	Scalar minDist2=Math::Constants<Scalar>::max;
	const StoredPointListItem* result=root.findClosestPoint(rootCenter,rootSize,queryPoint,minDist2);
	if(result==0)
		throw NoClosePointFoundError(queryPoint);
	
	return result->point;
	}

template <class StoredPointParam>
inline
typename PointTwoNTree<StoredPointParam>::ClosePointSet&
PointTwoNTree<StoredPointParam>::findClosestPoints(
	const typename PointTwoNTree<StoredPointParam>::Point& queryPoint,
	typename PointTwoNTree<StoredPointParam>::ClosePointSet& closestPoints) const
	{
	root.findClosestPoints(rootCenter,rootSize,queryPoint,closestPoints);
	return closestPoints;
	}

}
