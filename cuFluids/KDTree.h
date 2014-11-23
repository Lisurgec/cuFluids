#pragma once

// This is a KDTree implementation.  It's meant to be 
// instantiated and initialized on the CPU and then
// pushed to the GPU after.  

#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "Point.h"
#include "Box.h"

#include <iostream>
#include <thread>
#include <vector>

#define DIMENSIONS 3

class KDTree
{
	public:
	//Constructors
	KDTree();
	KDTree(Point3D* list, int numParticles);
	~KDTree();

	//Methods
	void insert(Point3D* point);
	Point3D getPoint();
	Box<float> getNode();
	Point3D getNodeValue(Box<float> node);
	void medianSortNodes();
	void validate();
	Point3D* findKClosestPointIndices(Point3D* p);
	int* findPointIndicesInRegion(Box<float>* b);
	Point3D* queryClosestPoints(Point3D* p);
	Point3D* queryClosestValues(Point3D* p);
	Point3D* queryKPoints(Point3D** p);
	std::vector<Point3D> KDTree::flatten();

	friend std::ostream& operator<<(std::ostream& out, KDTree& kd);

	private:
	Point3D root;

	std::vector<Box<float>> boxes;
	std::vector<Point3D> points;  //Vectors are guaranteed contiguous.

	void _insert(Point3D point, Point3D *root);
	static void _bfs(Point3D *point, std::vector<Point3D> *v);
};

KDTree::KDTree(Point3D* list, int numParticles)
{
	root = Point3D(0, 0, 0);
	for (int i = 0; i < numParticles; i++)
	{
		points.push_back(list[i]);
	}
};

KDTree::KDTree()
{
	root = Point3D(0, 0, 0);
}

KDTree::~KDTree()
{
	//this needs to cuda free everything too
	points.clear();
};

void KDTree::insert(Point3D *point)
{
	int k = 0;
	std::cout << *point << std::endl;

	if (*point < root)
	{
		if (root.left == nullptr)
		{
			root.left = point;
		}
		else
		{
			_insert(*point, root.left);
		}
	}

	else if (*point > root)
	{
		if (root.right == nullptr)
		{
			root.right = point;
		}
		else
		{
			_insert(*point, root.right);
		}
	}
}

void KDTree::_insert(Point3D point, Point3D *currNode)
{

	if (currNode == nullptr)
	{
		std::cout << "Null";
		std::cout << currNode << std::endl;
		return;
	}

	currNode->currentDimension = (currNode->currentDimension + 1) % 3;
	if (&point < currNode)
	{
		if (currNode->left == nullptr)
		{
			currNode->left = &point;
		}
		else
		{
			_insert(point, currNode->left);
		}
	}
	else if (&point > currNode)
	{
		if (currNode->right == nullptr)
		{
			currNode->right = &point;
		}
		else
		{
			_insert(point, currNode->right);
		}
	}
}

void KDTree::_bfs(Point3D *point, std::vector<Point3D> *v)
{

	std::thread* t1 = nullptr;
	std::thread* t2 = nullptr;

	v->push_back(*point);

	if (point->left != nullptr)
	{
		t1 = new std::thread(&KDTree::_bfs, point->right, v);
	}

	if (point->right != nullptr)
	{
		t2 = new std::thread(&KDTree::_bfs, point->right, v);
	}

	if (t1)
	{
		t1->join();
		delete t1;
	}

	if (t2)
	{
		t2->join();
		delete t2;
	}

}

std::vector<Point3D> KDTree::flatten()
{
	std::vector<Point3D> p; //Create an empty vector
	p.push_back(root); // Add our node
	std::thread* t1 = nullptr;
	std::thread* t2 = nullptr;

	if (root.left != nullptr)
	{
		t1 = new std::thread(&KDTree::_bfs, root.right, &p);
	}

	if (root.right != nullptr)
	{
		t2 = new std::thread(&KDTree::_bfs, root.right, &p);

	}
	if (t1->joinable())
	{
		t1->join();
		delete t1;
	}
	if (t2->joinable())
	{
		t2->join();
		delete t2;
	}
	return p;
}
