/***********************************************************************
PointOctree - Class to store three--dimensional points in an octree.
Copyright (c) 2003-2005 Oliver Kreylos

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

#define GEOMETRY_POINTOCTREE_IMPLEMENTATION

#include <Misc/PriorityHeap.h>

#include <Geometry/PointOctree.h>

namespace Geometry {

/**********************************
Methods of class PointOctree::Node:
**********************************/

template <class ScalarParam,class StoredPointParam>
inline
int
PointOctree<ScalarParam,StoredPointParam>::Node::splitPoints(
	int direction,
	typename PointOctree<ScalarParam,StoredPointParam>::Scalar mid,
	int numPoints,
	typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint* points)
	{
	/* Perform a Quicksort median step over the array to split the points according to mid: */
	int l=0;
	int r=numPoints-1;
	while(l<=r)
		{
		/* All points <l are <mid: */
		while(l<numPoints&&points[l][direction]<mid)
			++l;
		
		/* All points >r are >=mid: */
		while(r>=0&&points[r][direction]>=mid)
			--r;
		
		/* Swap if necessary: */
		if(l<r)
			{
			StoredPoint temp=points[l];
			points[l]=points[r];
			points[r]=temp;
			++l;
			--r;
			}
		}
	
	/* Return the number of points <mid: */
	return l;
	}

template <class ScalarParam,class StoredPointParam>
inline
void
PointOctree<ScalarParam,StoredPointParam>::Node::initialize(
	const typename PointOctree<ScalarParam,StoredPointParam>::Traversal& t,
	int sNumPoints,
	typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint* sPoints,
	int maxNumPoints,
	int maxDepth)
	{
	int i;
	/* Associate the given points: */
	numPoints=sNumPoints;
	points=sPoints;
	
	if(numPoints<=maxNumPoints||maxDepth==0)
		{
		/* Flag us a leaf: */
		children=0;
		}
	else
		{
		/* Split the points in the three directions: */
		int split[9];
		split[0]=0;
		split[8]=numPoints;
		
		/* Split in z direction: */
		split[4]=splitPoints(2,t.center[2],split[8]-split[0],points+split[0])+split[0];
		
		/* Split in y direction: */
		for(i=2;i<=6;i+=4)
			split[i]=splitPoints(1,t.center[1],split[i+2]-split[i-2],points+split[i-2])+split[i-2];
		
		/* Split in x direction: */
		for(i=1;i<=7;i+=2)
			split[i]=splitPoints(0,t.center[0],split[i+1]-split[i-1],points+split[i-1])+split[i-1];
		
		/* Associate the eight subarrays with the children: */
		children=new Node[8];
		for(i=0;i<8;++i)
			children[i].initialize(t.getChild(i),split[i+1]-split[i],points+split[i],maxNumPoints,maxDepth-1);
		}
	}

template <class ScalarParam,class StoredPointParam>
inline
const typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint*
PointOctree<ScalarParam,StoredPointParam>::Node::findClosePoint(
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& p,
	const typename PointOctree<ScalarParam,StoredPointParam>::Traversal& t) const
	{
	if(children==0) // We are a leaf
		{
		if(numPoints==0) // If there are no points inside this node, bail out: */
			return 0;
		
		/* Return the closest point inside the leaf: */
		const StoredPoint* closest=points;
		Scalar minDist2=sqrDist(*closest,p);
		for(int i=1;i<numPoints;++i)
			{
			Scalar dist2=sqrDist(points[i],p);
			if(dist2<minDist2)
				{
				closest=&points[i];
				minDist2=dist2;
				}
			}
		return closest;
		}
	else // We are an interior node
		{
		/* Find the child node containing the query point: */
		int childIndex=0x0;
		for(int i=0;i<3;++i)
			if(p[i]>=t.center[i])
				childIndex|=1<<i;
		
		return children[childIndex].findClosePoint(p,t.getChild(childIndex));
		}
	}

template <class ScalarParam,class StoredPointParam>
inline
void
PointOctree<ScalarParam,StoredPointParam>::Node::gatherStatistics(
	int& numNodes,
	int& numLeaves,
	int& maxNumPoints,
	int &depth,
	int nodeDepth) const
	{
	++numNodes;
	if(children==0)
		{
		++numLeaves;
		if(numPoints>maxNumPoints)
			maxNumPoints=numPoints;
		if(nodeDepth>depth)
			depth=nodeDepth;
		}
	else
		{
		for(int i=0;i<8;++i)
			children[i].gatherStatistics(numNodes,numLeaves,maxNumPoints,depth,nodeDepth+1);
		}
	}

/****************************************
Methods of class PointOctree::QueueEntry:
****************************************/

template <class ScalarParam,class StoredPointParam>
inline
PointOctree<ScalarParam,StoredPointParam>::QueueEntry::QueueEntry(
	const typename PointOctree<ScalarParam,StoredPointParam>::Traversal& sTraversal,
	const typename PointOctree<ScalarParam,StoredPointParam>::Node* sNode,
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& point)
	:minDist(0),traversal(sTraversal),node(sNode)
	{
	/* Find the space region the point is in and calculate the minimum distance: */
	Scalar d;
	for(int i=0;i<3;++i)
		{
		if((d=traversal.center[i]-traversal.size[i]-point[i])>PointOctree<ScalarParam,StoredPointParam>::PointScalarType(0))
			minDist+=d*d;
		else if((d=point[i]-traversal.center[i]-traversal.size[i])>PointOctree<ScalarParam,StoredPointParam>::PointScalarType(0))
			minDist+=d*d;
		}
	}

/****************************
Methods of class PointOctree:
****************************/

template <class ScalarParam,class StoredPointParam>
inline
PointOctree<ScalarParam,StoredPointParam>::PointOctree(
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& min,
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& max,
	int sNumPoints,
	typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint* sPoints,
	int maxNumPoints,
	int maxDepth)
	:numPoints(sNumPoints),points(sPoints),rootTraversal(mid(min,max),max-mid(min,max)),
	 root(new Node)
	{
	root->initialize(rootTraversal,numPoints,points,maxNumPoints,maxDepth);
	}

template <class ScalarParam,class StoredPointParam>
inline
PointOctree<ScalarParam,StoredPointParam>::~PointOctree(
	void)
	{
	delete[] points;
	delete root;
	}

template <class ScalarParam,class StoredPointParam>
inline
void
PointOctree<ScalarParam,StoredPointParam>::clear(
	void)
	{
	delete[] points;
	points=0;
	delete root;
	root=0;
	}

template <class ScalarParam,class StoredPointParam>
inline
void
PointOctree<ScalarParam,StoredPointParam>::setPoints(
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& min,
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& max,
	int sNumPoints,
	typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint* sPoints,
	int maxNumPoints,
	int maxDepth)
	{
	/* Clear the current tree: */
	delete[] points;
	delete root;
	
	/* Set the new tree: */
	numPoints=sNumPoints;
	points=sPoints;
	Point center=mid(min,max);
	rootTraversal=Traversal(center,max-center);
	root=new Node;
	root->initialize(rootTraversal,numPoints,points,maxNumPoints,maxDepth);
	}

template <class ScalarParam,class StoredPointParam>
inline
const typename PointOctree<ScalarParam,StoredPointParam>::StoredPoint&
PointOctree<ScalarParam,StoredPointParam>::findClosestPoint(
	const typename PointOctree<ScalarParam,StoredPointParam>::Point& p) const
	{
	const StoredPoint* bestPoint=points; // The current best point
	Scalar bestDist=sqrDist(*bestPoint,p); // The current best distance
	
	/* Put the root node into the queue: */
	Misc::PriorityHeap<QueueEntry> queue(1024);
	queue.insert(QueueEntry(rootTraversal,root,p));
	while(!queue.isEmpty())
		{
		/* Get the next element from the queue: */
		QueueEntry entry=queue.getSmallest();
		
		if(entry.minDist>=bestDist) // Early termination - the closest entry is already too far away
			break;
		
		queue.removeSmallest();
		
		if(entry.node->children==0) // The node is a leaf
			{
			/* Check all points inside the leaf: */
			for(int i=0;i<entry.node->numPoints;++i)
				{
				Scalar dist=sqrDist(entry.node->points[i],p);
				if(bestDist>dist)
					{
					bestPoint=&entry.node->points[i];
					bestDist=dist;
					}
				}
			}
		else // The node is an interior node
			{
			/* Visit all children of the node: */
			for(int i=0;i<8;++i)
				{
				/* Create a queue entry for the child: */
				QueueEntry childEntry(entry.traversal.getChild(i),&entry.node->children[i],p);

				if(childEntry.minDist<bestDist) // Does the entry possibly contain a better point?
					queue.insert(childEntry);
				}
			}
		}
	
	return *bestPoint;
	}

template <class ScalarParam,class StoredPointParam>
inline
void
PointOctree<ScalarParam,StoredPointParam>::gatherStatistics(
	int& numNodes,
	int& numLeaves,
	int& maxNumPoints,
	int& depth) const
	{
	numNodes=0;
	numLeaves=0;
	maxNumPoints=0;
	depth=0;
	root->gatherStatistics(numNodes,numLeaves,maxNumPoints,depth,0);
	}

}
