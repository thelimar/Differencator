#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>

#define DEBUG

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


struct Tree* TreeBuild (TreeData root_data, int root_type);
int AddNodeLeft (struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...);
int AddNodeRight (struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...);
void TreeGraphicDump (struct Tree* dis);
void DrawNodes (FILE* output, struct Node* node, struct Tree* eval_tree);
void NodeDestruct (struct Node* dis);
void TreeDestruct (struct Tree* dis);
int TreeVerificator (struct Tree* dis);
void TreeParentCheck (struct Node* dis, int* parent_ok);
void TreeTextDump (struct Node* dis, struct Tree* dis_tree);
void TreeCount (struct Node* dis, int* nNodes);
int TreeSize(struct Node* dis);
struct Node* CreateNode (struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type);
struct Node* CreateNodeCopy (struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type);
struct Node* NodeCopy (struct Node* node);
int TreesEql (struct Node* node1, struct Node* node2);
void NodeEql (struct Node* node1, struct Node* node2, int* val);

void BufferPutc (struct Buffer* dis, char c);
void BufferDestruct (struct Buffer* dis);
void BufferBuild (struct Buffer* dis);
void BufferPuts (struct Buffer* dis, const char* str);

void NodeBufferPut (struct NodeBuffer* dis, struct Node* node);
void NodeBufferDestruct (struct NodeBuffer* dis);
void NodeBufferBuild (struct NodeBuffer* dis);

/*
int main()
{

	struct Tree* tree1 = TreeBuild(ADD, FUNC);
	
	AddNodeLeft(tree1, tree1->root, 2.45, CONST);

	char x[] = "x";
	AddNodeRight(tree1, tree1->root, 0, VAR, x, NAN);

	TreeTextDump(tree1->root, tree1);


	return 0;
}
*/

struct Node* CreateNodeCopy (struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type)
{
	struct Node* dis_node = (struct Node*) calloc (1, sizeof(struct Node));

	DBG printf ("copy data %lf type %d\n", node_data, node_type);

	if (dis_node)
		dis_node->data = node_data;

	if (left)
	{
		dis_node->left = CreateNodeCopy (dis, left->left, left->right, left->data, left->type);
	}
	else
		dis_node->left = NULL;

	if (right)
	{
		dis_node->right = CreateNodeCopy (dis, right->left, right->right, right->data, right->type);
	}
	else
		dis_node->right = NULL;

	dis_node->parent = NULL;
	dis_node->type = node_type;

	dis->size++;

	return dis_node;
}

struct Node* NodeCopy(Node* node)
{
	struct Node* node_copy = (struct Node*)calloc(1, sizeof(struct Node));

	node_copy->data = node->data;
	node_copy->left = node->left;
	node_copy->right = node->right;
	node_copy->parent = node->parent;
	node_copy->type = node->type;

	return node_copy;
}

int TreesEql (Node* node1, Node* node2)
{
	int eql = 1;
	NodeEql(node1, node2, &eql);
	return eql;
}

void NodeEql (struct Node* node1, struct Node* node2, int* val)
{
	if (node1->type != node2->type || node1->data != node2->data)
	{
		*val = 0;
		return;
	}

	if (node1->left && node2->left)
		NodeEql (node1->left, node2->left, val);
	else if (node1->left || node2->left)
	{
		*val = 0;
		return;
	}
	
	if (node1->right && node2->right)
		NodeEql(node1->right, node2->right, val);
	else if (node1->right || node2->right)
	{
		*val = 0;
		return;
	}
}

struct Node* CreateNode (struct Tree* dis, struct Node* left, struct Node* right, TreeData node_data, int node_type)
{
	struct Node* dis_node = (struct Node*) calloc (1, sizeof(struct Node));

	dis_node->data = node_data;

	dis_node->left = left;
	dis_node->right = right;

	dis_node->parent = NULL;
	dis_node->type = node_type;

	dis->size++;

	return dis_node;
}


void TreeParentCheck(struct Node* dis, int* parent_ok)
{
	if (dis->left)
	{
		if (dis->data != dis->left->parent->data)
		{
			*parent_ok = 0;
			printf("fail parent check on %lf", dis->data);
			return;
		}
		TreeParentCheck(dis->left, parent_ok);
	}
	if (dis->right)
	{
		if (dis->data != dis->right->parent->data)
		{
			*parent_ok = 0;
			printf("fail parent check on %lf", dis->data);
			return;
		}
		TreeParentCheck(dis->right, parent_ok);
	}
	return;
}

int TreeSize (struct Node* dis)
{
	int size = 0;

	TreeCount(dis, &size);

	return size;
}

void TreeCount (struct Node* dis, int* nNodes)
{
	(*nNodes)++;
	if (dis->left)
		TreeCount(dis->left, nNodes);
	if (dis->right)
		TreeCount(dis->right, nNodes);
}

int TreeVerificator(struct Tree* dis)
{
	assert(dis);

	int error = 0;

	if (dis->size < 1)
		error = NEG_SIZE_ERROR;
	else if (!dis->root)
		error = ROOT_NULL;
	else if (true)
	{
		int nNodes = 0;
		TreeCount(dis->root, &nNodes);
		if (dis->size != nNodes)
			error = SIZE_ERROR;
	}

	else if (dis->size >= 2)
	{
		int parent_ok = 1;
		TreeParentCheck(dis->root, &parent_ok);
		if (!parent_ok)
			error = RELATION_ERROR;
	}

	if (error != 0)
		printf("ERROR #%d\n", error);

	return error;
}

void BufferBuild(struct Buffer* dis)
{
	dis->data = (char*)calloc(MAX_VARS, sizeof(char));
	dis->len = 0;
}

void BufferDestruct(struct Buffer* dis)
{
	free(dis->data);
	dis->len = 0;
}

void BufferPutc(struct Buffer* dis, char c)
{
	dis->data[dis->len] = c;
	(dis->len)++;
}

void BufferPuts (struct Buffer* dis, const char* str)
{
	for (int i = 0; str[i]; i++)
	{
		dis->data[dis->len] = str[i];
		(dis->len)++;
	}
}

void NodeBufferPut (NodeBuffer* dis, Node* node)
{
	dis->data[dis->len] = node;
	(dis->len)++;
}

void NodeBufferDestruct (struct NodeBuffer* dis)
{
	for (int i = 0; i < dis->len; i++)
		free(dis->data[i]);

	free(dis->data);
	dis->len = 0;
}

void NodeBufferBuild (struct NodeBuffer* dis)
{
	dis->data = (struct Node**) calloc (MAX_CHANGES, sizeof(struct Node*));
	dis->len = 0;
}

struct Tree* TreeBuild (TreeData root_data, int root_type)
{
	TreeData Root_data = root_data;

	struct Tree* tree = (struct Tree*) calloc(1, sizeof(Tree));
	if (tree)
	{
		struct Buffer var_names_buffer{};
		BufferBuild (&var_names_buffer);

		tree->var_names = var_names_buffer;
		tree->var_values = (double*) calloc (MAX_VARS, sizeof(double));

		if (tree->var_values)
			for (int i = 0; i < MAX_VARS; i++)
				tree->var_values[i] = NAN;

		tree->size = 1;
		tree->root = (struct Node*) calloc (1, sizeof(struct Node));

		if (tree->root)
		{
			tree->root->data = Root_data;
			tree->root->type = root_type;
			tree->root->left = 0;
			tree->root->right = 0;
			tree->root->parent = 0;
		}
		else
			return NULL;
	}
	else
		return NULL;

	struct NodeBuffer node_buffer{};

	NodeBufferBuild (&node_buffer);

	tree->change_buffer = node_buffer;

	return tree;
}

void NodeDestruct (struct Node* dis)
{
	if (dis->left)
	{
		NodeDestruct (dis->left);
	}
	if (dis->right)
	{
		NodeDestruct (dis->right);
	}

	free (dis);
	dis = NULL;
	return;
}

void TreeDestruct (struct Tree* dis)
{
	NodeDestruct (dis->root);
	dis->size = 0;

	BufferDestruct (&dis->var_names);
	free (dis->var_values);

	NodeBufferDestruct(&dis->change_buffer);

	free (dis);
	dis = nullptr;
}

int AddNodeLeft (struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...)
{
	IsOK(dis)

	if (!parent->left)
	{
		struct Node* node = (struct Node*)calloc (1, sizeof(struct Node));
		if (node)
		{
			node->data = node_data;
			node->left = 0;
			node->parent = parent;
			node->right = 0;
			node->type = node_type;

			if (node_type == VAR)
			{
				va_list var_params;
				va_start(var_params, node_type);

				dis->var_names.data[(int) node_data] = va_arg(var_params, char);
				dis->var_values[(int) node_data] = va_arg(var_params, double);

				va_end(var_params);
			}

			(dis->size)++;
			parent->left = node;
		}
		return 0;
	}
	else
	{
		AddNodeLeft (dis, parent->left, node_data, node_type);
	}
	printf ("Add error\n");
	return 1;
}

int AddNodeRight (struct Tree* dis, struct Node* parent, TreeData node_data, int node_type, ...)
{
	IsOK(dis)

	if (!parent->right)
	{

		struct Node* node = (struct Node*)calloc (1, sizeof(struct Node));
		if (node)
		{
			node->data = node_data;
			node->left = 0;
			node->parent = parent;
			node->right = 0;
			node->type = node_type;

			if (node_type == VAR)
			{
				va_list var_params;
				va_start(var_params, node_type);

				dis->var_names.data[(int) node_data] = va_arg(var_params, char);
				dis->var_values[(int) node_data] = va_arg(var_params, double);

				va_end(var_params);
			}

			(dis->size)++;
			parent->right = node;
		}
		return 0;
	}
	else
	{
		AddNodeRight (dis, parent->left, node_data, node_type);
	}
	printf ("Add error\n");
	return 1;
}


void TreeTextDump (struct Node* dis, struct Tree* dis_tree)
{
	if (dis->left)
	{
		printf ("(");
		TreeTextDump (dis->left, dis_tree);
	}

	switch (dis->type)
	{
		case CONST_T: printf("%lf", dis->data);										     break;
		case VAR:   printf("%c", dis_tree->var_names.data[(int) dis->data]);             break;
		case FUNC:
			switch ((int)dis->data)
			{
			case ADD: printf("+");                                break;
			case MUL: printf("*");                                break;
			case SUB: printf("-");                                break;
			case DIV: printf("/");                                break;
			case SQRT: printf("sqrt");                            break;
			case POW: printf("pow");                              break;
			case SIN: printf("sin");                              break;
			case COS: printf("cos");                              break;
			case TAN: printf("tan");                              break;
			case LOG: printf("log");                              break;
			case LN: printf("ln");                                break;
			}                                                                            break;
	}

	if (dis->right)
	{
		printf ("(");
		TreeTextDump (dis->right, dis_tree);
	}

	printf (")");
	return;
}

#define GraphOut(func) fprintf(output, "peak%p [label=\"%s\"];\n", node, func);

void DrawNodes(FILE* output, struct Node* node, struct Tree* eval_tree)
{
	switch (node->type)
	{
	case CONST_T: fprintf (output, "peak%p [label=\"%lf\"];\n", node, node->data);								           break;
	case VAR:   fprintf (output, "peak%p [label=\"%c\"];\n", node, eval_tree->var_names.data[(int)node->data]);            break;
	case FUNC:
		switch ((int)node->data)
		{
		case ADD: GraphOut("+")                                                                     break;
		case MUL: GraphOut("*")                                                                     break;
		case SUB: GraphOut("-")                                                                     break;
		case DIV: GraphOut("/")                                                                     break;
		case SQRT: GraphOut("sqrt")                                                                 break;
		case POW: GraphOut("^")                                                                     break;
		case SIN: GraphOut("sin")                                                                   break;
		case COS: GraphOut("cos")                                                                   break;
		case TAN: GraphOut("tan")                                                                   break;
		case LOG: GraphOut("log")                                                                   break;
		case LN: GraphOut("ln")                                                                     break;
		default: GraphOut("UNKNOWN")                                                                break;
		}                                                                                                                          break;
	default: printf("Unknown type %d", node->type);                                                                                return;
	}

	if (node->left)
	{
		fprintf (output, "peak%p -> peak%p\n", node, node->left);
		DrawNodes (output, node->left, eval_tree);
	}
	if (node->right)
	{
		fprintf (output, "peak%p -> peak%p\n", node, node->right);
		DrawNodes (output, node->right, eval_tree);
	}

	return;
}

void TreeGraphicDump(struct Tree* dis)
{
	FILE* Graph = fopen("Graph.dot", "w");
	fprintf (Graph, "digraph G{\nrankdir=TB;\n");

	fprintf (Graph, "edge[color = \"darkorange\"];\n");
	fprintf (Graph, "node [style = \"filled\", shape=record, fillcolor=\"darkkhaki\"];\n");

	DrawNodes (Graph, dis->root, dis);

	fprintf (Graph, "}");

	fclose (Graph);

	system ("D:\\Program_Files\\ATT\\Graphviz\\bin\\dot.exe -Tpng D:\\progi\\Differencator\\Graph.dot -o D:\\progi\\Differencator\\GraphDump.png");
	system ("start D:\\progi\\Differencator\\GraphDump.png");
}