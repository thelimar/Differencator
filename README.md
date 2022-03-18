# Differencator
A program that can calculate derivative of pretty much any function with some interesting features.

# Table of contents
1. [Introduction](#intro)
2. [Implementation](#real)
3. [LaTeX dump](#latex)
4. [Expression simplification](#simple)
5. [Lexical analysis](#lexic)

<a name="intro"></a>
## 1. Introduction

I have made a program that can find derivative of functions and added some non-trivial features that you might find interesting. :)

<a name="real"></a>
## 2. Implementation

The main structure I used in this project is EvalTree - my own version of tree designed for this problem. Basically, the main difference is that each node now has one of four types: CONST_T, VAR, FUNC, CHANGE. The first three are pretty much self-explanatory and the last one is used for optimization that I will talk about later. This makes working with input expressions much easier. Other differences from a traditional tree structure are connected with storing some extra information like names and values of variables and changeable tree parts, which I will talk more about in [simplification](#simple) section.
To make the main function responsible for calculation derivative more readable, more understandable for humans (and more beautiful) I used a kind of DSL (yes, using defines, don't judge me) that allows you to work with tree nodes as you are working with numbers, variables and functions, as we are used to in math. Here is a code fragment with it:
```
#define D(node) Diff (node, eval_tree, str_buffer)
#define C(node) CreateNodeCopy (eval_tree, node->left, node->right, node->data, node->type)
#define Const(value) CreateNode (eval_tree, NULL, NULL, value, CONST_T)
#define TeX TeXTransform (eval_tree, node, diffed_node, str_buffer)

#define Addition(left, right) CreateNode (eval_tree, left, right, ADD, FUNC)
#define Multiply(left, right) CreateNode (eval_tree, left, right, MUL, FUNC)
#define Subt(left, right)     CreateNode (eval_tree, left, right, SUB, FUNC)
#define Division(left, right) CreateNode (eval_tree, left, right, DIV, FUNC)
#define Power(left, right)    CreateNode (eval_tree, left, right, POW, FUNC)
#define SQR(right)            CreateNode (eval_tree, right, CreateNode (eval_tree, NULL, NULL, 2, CONST_T), POW, FUNC)
#define SQRTed(right)         CreateNode (eval_tree, CreateNode (eval_tree, NULL, NULL, 0.5, CONST_T), right, POW, FUNC)
#define A_log(left, right)    CreateNode (eval_tree, left, right, LOG, FUNC)
#define Sin(right)            CreateNode (eval_tree, NULL, right, SIN, FUNC)
#define Cos(right)            CreateNode (eval_tree, NULL, right, COS, FUNC)
#define Tan(right)            CreateNode (eval_tree, NULL, right, TAN, FUNC)
#define Ln(right)             CreateNode (eval_tree, NULL, right, LN, FUNC)

struct Node* Diff (struct Node* node, struct Tree* eval_tree, struct Buffer* str_buffer)
{
	DBG printf ("current size %d\n", eval_tree->size);
	
	struct Node* diffed_node = node;

	switch (node->type)
	{
	case CONST_T: diffed_node = CreateNode (eval_tree, NULL, NULL, 0, CONST_T);                           TeX; return diffed_node;
	case VAR:     diffed_node = CreateNode (eval_tree, NULL, NULL, 1, CONST_T);                           TeX; return diffed_node;
	case FUNC:
		switch ((int) node->data)
		{
		case ADD: diffed_node =  Addition (D(L), D(R));                                                   TeX; return diffed_node;
		case MUL: diffed_node = Addition (Multiply (D(L), C(R)), Multiply (C(L), D(R)));                  TeX; return diffed_node;
		case SUB: diffed_node = Subt (D(L), D(R));                                                        TeX; return diffed_node;
		case DIV: diffed_node = Division (Subt (Multiply (D(L), C(R)), Multiply (C(L), D(R))), SQR (C(R)));  TeX; return diffed_node;
		case SQRT: diffed_node = Multiply (Division (Const(0.5), SQRTed (C(R))), D(R));                   TeX; return diffed_node;
		case POW: 
			if (!TreeIsVar (L))
			{
				diffed_node = Multiply (Multiply(Power(C(L), C(R)), Ln(C(L))), D(R));                     TeX; return diffed_node;
			}
			else if (!TreeIsVar (R))
			{
				diffed_node = Multiply (Multiply(Power(C(L), Subt(C(R), Const(1))), C(R)), D(L));         TeX; return diffed_node;
			}
			else
			{
				diffed_node = Multiply (Power(Const(e), Multiply(C(R), Ln(C(L)))),
					                   D(Multiply(C(R), Ln(C(L)))));                                      TeX; return diffed_node;
			}
		case SIN: diffed_node = Multiply (Cos (C(R)), D(R));                                              TeX; return diffed_node;                                                           
		case COS: diffed_node = Multiply (Subt (Const(0), Sin(C(R))), D(R));                              TeX; return diffed_node;                                               
		case TAN: diffed_node = Multiply (Tan (C(R)), D(R));                                              TeX; return diffed_node;
		case LOG: diffed_node = Division (D(R), Multiply (C(R), Ln (C(L))));                              TeX; return diffed_node;                                        
		case LN:  diffed_node = Division (D(R), C(R));                                                    TeX; return diffed_node;                                                             
		default:  printf ("Unknown func %d", (int)node->data);                                                return 0;
		}
	case CHANGE:
		return D(eval_tree->change_buffer.data[(int) node->data]);                                                                                       
	default: printf ("Unknown type %d", node->type);                                                          return 0;
	}
}
```
<a name="latex"></a>
## 3. LaTeX dump

Debugging of a program is an essential part of developing it, so why not make it hilarious for once?) In this project, while having a graph dump using GraphViz, which I have used before, I have decided to implement a PDF dump in a form of laboratory work report using LaTeX. It's much easier to show it rather than explain, so [here](https://github.com/thelimar/Differencator/blob/main/Laba.pdf) is an example. Essentially, each time the main expression tree is changed I dump the current state of it in form of a LaTeX formula. Then I mix it up with some puns and in the end we have a pdf file.

<a name="simple"></a>
## 4. Expression simplification

The main problem of a method of calculating derivative used in this project I stumbled upon is the fact that resulting expressions look unnatural. There is a lot of things to be done with expression to make it look more math-appropriate. So, this was my next goal and to achieve it I made to functions: ConstantFolding and NeutralFolding. The first one replaces three nodes with one if two leaves out of three are constants with one node with the result of the operation in the third node. The second is a little more complex. It also scans groups of three nodes in search of ones where the operation node has its neutral element as one of the operands. For example, neutral element for sum is 0, as the group of three nodes two of which are + and 0 can easily be replaced with one node containing the data of the third node. It is important to take into account that there are operations where the order of the operands matters. Here is the result of simplification:

Unoptimized:
![Unoptimized](https://github.com/thelimar/Differencator/blob/main/GraphDumps/GraphDumpUnoptimized.png)

Optimized:
![Optimized](https://github.com/thelimar/Differencator/blob/main/GraphDumps/GraphDumpOptimized.png)

Furthermore, to make my PDF dump look more like a lab work report I implemented a feature that seeks for large enough recurring portions of the expression tree and replaces them with a special node type that is represented as a single letter. That way resulting expression looks more natural.

<a name="lexic"></a>
## 5. Lexical analysis

Lastly, I wanted my program to understand the mathematical priority of operations by itself without a user constantly putting brackets in the input string. For this case I have used basic lexical analysis determining what the lexemes of my input string are and making a hierarchy of handler functions. Basically, the main idea is that each function is responsible for handling a certain type of lexemes from the same priority group. When a function currently responsible for handling the input string stumbles upon a lexeme that is out of its priority group it calls for the function one level of priority up. The function hierarchy is designed in a way that functions that are responsible for the least priority lexemes are called first, yet they return last, because actual handling happens after all functions from upper priority group has returned.
