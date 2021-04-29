#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

//#define DEBUG

#ifndef DEBUG
#define DBG if(0)
#else
#define DBG
#endif

#define IsOK(Tree) int error = TreeVerificator(Tree); if (error != 0) {TreeGraphicDump(Tree); return error;}

typedef double TreeData;

enum Eval_Tree_Parametors
{
	MAX_VARS = 20,
	MAX_FUNC_NAME = 10,
	MAX_CHANGES = 20,
};

enum Errors
{
	SIZE_ERROR = 1,
	RELATION_ERROR = 2,
	ROOT_NULL = 3,
	NEG_SIZE_ERROR = 4,
};

enum Types
{
	CONST_T = 1,
	VAR = 2,
	FUNC = 3,
	CHANGE = 4,
};

enum Funcs
{
	PREFIX_OPER = 0x80,

	ADD = 1,
	MUL = 2,
	SUB = 3,
	DIV = 4 | PREFIX_OPER,
	SQRT = 5,
	POW = 6,
	SIN = 7,
	COS = 8,
	TAN = 9,
	LOG = 10 | PREFIX_OPER,
	LN = 11,
};

struct Buffer
{
	char* data;
	int len;
};

struct Node
{
	TreeData data;
	int type;
	struct Node* left;
	struct Node* right;
	struct Node* parent;
};

struct NodeBuffer
{
	Node** data;
	int len;
};

struct Tree
{
	struct NodeBuffer change_buffer;
	struct Node* root;
	int size;
	struct Buffer var_names;
	double* var_values;
};

struct Tree* TreeBuild(TreeData root_data, int root_type);
int AddNodeLeft(struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...);
int AddNodeRight(struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...);
void TreeGraphicDump(struct Tree* dis);
void DrawNodes(FILE* output, struct Node* node, struct Tree* eval_tree);
void NodeDestruct(struct Node* dis);
void TreeDestruct(struct Tree* dis);
int TreeVerificator(struct Tree* dis);
void TreeParentCheck(struct Node* dis, int* parent_ok);
void TreeTextDump(struct Node* dis, struct Tree* dis_tree);
void TreeCount(struct Node* dis, int* nNodes);
int TreeSize(struct Node* dis);
struct Node* CreateNode(struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type);
struct Node* CreateNodeCopy(struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type);
struct Node* NodeCopy(struct Node* node);
int TreesEql(struct Node* node1, struct Node* node2);
void NodeEql(struct Node* node1, struct Node* node2, int* val);

void BufferPutc(struct Buffer* dis, char c);
void BufferDestruct(struct Buffer* dis);
void BufferBuild(struct Buffer* dis);
void BufferPuts(struct Buffer* dis, const char* str);

void NodeBufferPut(struct NodeBuffer* dis, struct Node* node);
void NodeBufferDestruct(struct NodeBuffer* dis);
void NodeBufferBuild(struct NodeBuffer* dis);
