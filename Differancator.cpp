#define _CRT_SECURE_NO_WARNINGS

#include "D:\progi\Core i DED\Core i DED\TXLib.h"
#include "EvalTree.h"

#define Require(c) if (eval->eval_str[eval->pos] == c) eval->pos++;                                                                         \
                   else {printf ("Syntax error in %s\ncurrent str %s\n", __FUNCSIG__, eval->eval_str + eval->pos); return NULL;}            \

enum Parametors
{
	MAX_BUFFER_LEN = 10000,
	MAX_CONST_LEN = 10,
	PUNS = 10,
	MAX_PUN_LEN = 80,
	MAX_INPUT_STR = 80,

	MAX_OPTIMAL_CHANGE_SIZE = 10,
	MIN_OPTIMAL_CHANGE_SIZE = 7,

	SELF_REMOVE = 1,
	BROTHER_REMOVE = 0,
};

enum NeutralFold
{
	BOTH_SIDES = 0,
	ONE_SIDE = 1,
};

struct Eval
{
	char* eval_str;
	int pos;
};

#define streql(str1, str2) !strcmp (str1, str2)
#define a_log(a, n) log10 (n)/log10 (a)
#define e 2.7182818

struct Tree* ReadEvalTree (char* input_str);

void VarAssign (struct Tree* eval_tree, char var, double value);
int VarSearch (struct Tree* eval_tree, char var);

void TreeDifferencation(struct Tree* dis, struct Buffer* str_buffer);
struct Node* Diff(struct Node* node, struct Tree* eval_tree, struct Buffer* str_buffer);
double Eval (struct Node* node, struct Tree* eval_tree);

void NeutralFolding (struct Node* node, struct Tree* eval_tree);
void NeutralFold (int neutral_data, int one_side, int self_remove, struct Node* node, struct Tree* eval_tree);
void TreeOptimized (struct Tree* dis);
void ConstantFolding (struct Node* node, struct Tree* eval_tree);

struct Buffer TeXDumpBegin (struct Tree* dis);
void TeXDumpNode (struct Tree* dis_tree, struct Node* dis, struct Buffer* str_buffer);
void TeXDumpEnd (struct Buffer* str_buffer, FILE* output, struct Node* dis, struct Tree* dis_tree);
void TeXTransform (struct Tree* dis_tree, struct Node* old_node, struct Node* dis, struct Buffer* str_buffer);
void ParentBond (struct Node* node);
void NodeChange (struct Node* dis, struct NodeBuffer* change_buffer);

int TreeIsVar (struct Node* node);
void NodeIsVar (struct Node* node, int* is_var);

struct Node* GetG (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetE (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetT (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetN (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetV (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetP (struct Eval* eval, struct Tree* eval_tree);
struct Node* GetF (struct Eval* eval, struct Tree* eval_tree);


int main()
{
	txSetLocale (1251);

	char input_str[MAX_INPUT_STR] = "";

	int offset = 0;

	if (!scanf ("%[^\r\n]%n", input_str, &offset))
		return -1;

	input_str[offset] = '$';

	DBG printf ("input %s\n", input_str);

	struct Tree* eval_tree = ReadEvalTree (input_str);

	TreeGraphicDump (eval_tree);

	TreeTextDump (eval_tree->root, eval_tree);

	FILE* output = fopen ("Laba.tex", "wb");

	struct Buffer pdf_buffer = TeXDumpBegin (eval_tree);

	TreeDifferencation (eval_tree, &pdf_buffer);
	
	TreeGraphicDump (eval_tree);

	TreeOptimized (eval_tree);

	VarAssign (eval_tree, 'x', 3);
	printf ("\nresult %lf\n", Eval(eval_tree->root, eval_tree));

	TreeGraphicDump(eval_tree);

	TeXDumpEnd (&pdf_buffer, output, eval_tree->root, eval_tree);

	printf ("tree size %d\n", eval_tree->size);

	printf ("tree size %d\n", eval_tree->size);

	TreeDestruct (eval_tree);

	return 0;
}

#define L node->left
#define R node->right
#define E(node) Eval (node, eval_tree)

double Eval (struct Node* node, struct Tree* eval_tree)
{
	switch (node->type)
	{
	case CONST_T: return node->data;                                             							
	case VAR:   if (!isnan(eval_tree->var_values[(int)node->data])) return eval_tree->var_values[(int)node->data];
			    else printf ("var not assigned: var %c index %d\n", eval_tree->var_names.data[(int)node->data], (int)node->data);       
		             return 0xDEADBEEF;
	case FUNC:
		switch ((int)node->data)
		{
		case ADD: return E(L) + E(R);                                                                   
		case MUL: return E(L) * E(R);                                                  
		case SUB: return E(L) - E(R);                                                             
		case DIV: return E(L) / E(R);                                                          
		case SQRT: return sqrt (E(R));                                                         
		case POW: return pow (E(L), E(R));                                                         
		case SIN: return sin (E(R));                                                             
		case COS: return cos (E(R));                                                                  
		case TAN: return tan (E(R));                                                                  
		case LOG: return a_log (E(L), E(R));                                                      
		case LN: return log (E(R));                                                                 
		default:  printf ("Unknown func %d\n", (int)node->data); return 0;
		}                                                                                                           
	default: printf ("Unknown type %d", node->type);                                                                     return 0;
	}
}

void TreeDifferencation(struct Tree* dis, struct Buffer* str_buffer)
{
	dis->size = 0;

	struct Node* diffed_root = Diff (dis->root, dis, str_buffer);
	NodeDestruct (dis->root);

	dis->root = diffed_root;

	ParentBond (diffed_root);
}

int TreeIsVar (struct Node* node)
{
	int is_var = 0;

	NodeIsVar (node, &is_var);

	return is_var;
}

void NodeIsVar (struct Node* node, int* is_var)
{
	if (node->type == VAR)
	{
		*is_var = 1;
		return;
	}

	if (node->left)
		NodeIsVar (node->left, is_var);

	if (node->right)
		NodeIsVar (node->right, is_var);
}

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

struct Tree* ReadEvalTree (char* input_str)
{
	struct Tree* eval_tree = TreeBuild (1, CONST_T);
	eval_tree->size = 0;

	struct Eval eval = { input_str, 0 };

	eval_tree->root = GetG (&eval, eval_tree);

	ParentBond (eval_tree->root);

	return eval_tree;
}

void VarAssign (struct Tree* eval_tree, char var, double value)
{
	for (int i = 0; i < eval_tree->var_names.len; i++)
	{
		if (eval_tree->var_names.data[i] == var)
		{
			eval_tree->var_values[i] = value;
			DBG printf ("index %d, var %c\n", i, eval_tree->var_names.data[i]);
			return;
		}
	}
	printf ("Var not found\n");
}

int VarSearch (struct Tree* eval_tree, char var)
{
	for (int i = 0; i < eval_tree->var_names.len; i++)
	{
		if (eval_tree->var_names.data[i] == var)
			return i;
	}

	return -1;
}

void ConstantFolding (struct Node* node, struct Tree* eval_tree)
{
	if ((node->left && node->left->type == CONST_T) || !node->left)
	{
		if (node->right && node->right->type == CONST_T)
		{
			node->data = Eval (node, eval_tree);
			free (node->right);
			free (node->left);

			node->left = NULL;
			node->right = NULL;
			node->type = CONST_T;

			eval_tree->size -= 2;
		}
	}
	if (node->left && node->left->type == FUNC)
		ConstantFolding (node->left, eval_tree);

	if (node->right && node->type == FUNC)
		ConstantFolding (node->right, eval_tree);
}

void ParentBond (struct Node* node)
{
	if (node->left)
	{
		node->left->parent = node;
		ParentBond (node->left);
	}
	if (node->right)
	{
		node->right->parent = node;
		ParentBond (node->right);
	}
}

void NeutralFold (int neutral_data, int one_side, int self_remove, struct Node* node, struct Tree* eval_tree)
{
	if (node->left && node->right && node->left->type == CONST_T && node->left->data == neutral_data && !one_side)
	{
		if (self_remove)
		{
			free (node->left);
			if (node->parent && node->parent->left == node)
				node->parent->left = node->right;
			else if (node->parent && node->parent->right == node)
				node->parent->right = node->right;
			else
				eval_tree->root = node->right;
			node->right->parent = node->parent;
			eval_tree->size -= 2;
			free (node);
		}
		else
		{
			free (node->right);
			if (node->parent && node->parent->left == node)
				node->parent->left = node->left;
			else if (node->parent && node->parent->right == node)
				node->parent->right = node->left;
			else
				eval_tree->root = node->left;
			node->left->parent = node->parent;
			eval_tree->size -= 2;
			free (node);
		}

		node->right = NULL;
		node->left = NULL;
	}

	else if (node->right && node->left && node->right->type == CONST_T && node->right->data == neutral_data)
	{
		if (self_remove)
		{
			free (node->right);
			if (node->parent && node->parent->right == node)
				node->parent->right = node->left;
			else if (node->parent && node->parent->left == node)
				node->parent->left = node->left;
			else
				eval_tree->root = node->left;
			node->left->parent = node->parent;
			eval_tree->size -= 2;
			free (node);
		}
		else
		{
			free (node->left);
			if (node->parent && node->parent->left == node)
				node->parent->left = node->right;
			else if (node->parent && node->parent->right == node)
				node->parent->right = node->right;
			else
				eval_tree->root = node->right;
			node->right->parent = node->parent;
			eval_tree->size -= 2;
			free (node);
		}

		node->right = NULL;
		node->left = NULL;
	}
}

void NeutralFolding (struct Node* node, struct Tree* eval_tree)
{
	if (node->right && node->right->type == FUNC)
		NeutralFolding (node->right, eval_tree);
	if (node->left && node->left->type == FUNC)
		NeutralFolding (node->left, eval_tree);

	if (node->type == FUNC)
	{
		switch ((int)node->data)
		{
		case ADD: NeutralFold (0, BOTH_SIDES, SELF_REMOVE, node, eval_tree);                                              break;
		case MUL: NeutralFold (1, BOTH_SIDES, SELF_REMOVE, node, eval_tree);
			      NeutralFold (0, BOTH_SIDES, BROTHER_REMOVE, node, eval_tree);                                           break;
		case SUB: NeutralFold (0, ONE_SIDE, SELF_REMOVE, node, eval_tree);                                                break;
		case DIV: NeutralFold (1, ONE_SIDE, SELF_REMOVE, node, eval_tree);                                                break;
		case POW: NeutralFold (1, ONE_SIDE, SELF_REMOVE, node, eval_tree);                                                break;
		}
	}
}

void TreeOptimized (struct Tree* dis)
{
	int size_before = 0;

	do
	{
		size_before = dis->size;

		ConstantFolding (dis->root, dis);
		NeutralFolding (dis->root, dis);
	} 
	while (dis->size != size_before);
}

struct Buffer TeXDumpBegin (struct Tree* dis)
{
	_set_printf_count_output (1);

	struct Buffer str_buffer{};

	str_buffer.data = (char*)calloc (MAX_BUFFER_LEN, sizeof(char));
	str_buffer.len = 0;

	BufferPuts(&str_buffer, "\\documentclass[12pt,a4paper]{article}\r\n"
							"\\usepackage[T2A]{fontenc}\r\n"
							"\\usepackage[russian]{babel}\r\n"
							"\\usepackage[cp1251]{inputenc}\r\n"
							"\\usepackage[normalem]{ulem}\r\n"
							"\\usepackage{textcomp}\r\n"
							"\\usepackage{xcolor}\r\n"
							"\\usepackage{hyperref}\r\n"
							"\\definecolor{urlcolor}{HTML}{799B03}\r\n"
							"\\begin{document}\r\n"
							"\\begin{center}\r\n"
							"\\Large Отчет о выполнении лабораторной работы 1.1.3/4\r\n"
							"\\end{center}\r\n"
							"\\begin{center}\r\n"
							"\\LARGE {\\bf Взятие сложной производной}\\\\\r\n"
							"\\end{center}\r\n"
							"\\begin{center}\r\n"
							"\\normalsize Кафедра \\sout{дедовских наук} высшей математики МФТИ от 15.12.2020\r\n"
							"\\end{center}\r\n"
							"Взятие производных сложных функций, безусловно, является важнейшим достижением человеческой эволюции, и, несомненно, \r\n"
							"продемонстрировать дифференциирование на конкретном примере - это долг каждого обучающегося.\\\\\r\n"
							"\\section{Производная}\r\n"
							"Возьмем производную функции\r\n"
							"\\begin{equation}\r\n");

	TeXDumpNode (dis, dis->root, &str_buffer);
	BufferPuts (&str_buffer, "\r\n\\end{equation}\r\n");

	return str_buffer;
}

void TeXDumpNode (struct Tree* dis_tree, struct Node* dis, struct Buffer* str_buffer)
{
	int double_len = 0;

	if (dis->type == FUNC && ((int) dis->data & PREFIX_OPER) == PREFIX_OPER)
	{
		switch ((int) dis->data)
		{
		case DIV: BufferPuts (str_buffer, "\\frac{");                break;
		case LOG: BufferPuts (str_buffer, "\\log_{");                break;
		}

		TeXDumpNode (dis_tree, dis->left, str_buffer);
		BufferPuts (str_buffer, "}{");
		TeXDumpNode (dis_tree, dis->right, str_buffer);
		BufferPutc (str_buffer, '}');
	}
	else
	{
		if (dis->left && dis->type != CHANGE && !(dis->data == SUB && dis->left->data == 0))
		{
			if (dis->left->type == FUNC && dis->data != ADD && dis->data != SUB && 
			   (dis->left->data == ADD || dis->left->data == SUB) && dis->left->data != MUL)
			{
				BufferPutc (str_buffer, '(');
				TeXDumpNode (dis_tree, dis->left, str_buffer);
				BufferPutc (str_buffer, ')');
			}
			else
				TeXDumpNode (dis_tree, dis->left, str_buffer);
		}

		switch (dis->type)
		{
		case CONST_T: 
			if ((int) dis->data == dis->data)
			{
				sprintf(str_buffer->data + str_buffer->len, "%d%n", (int)dis->data, &double_len); 
				(str_buffer->len) += double_len;
			}
			else
			{
				sprintf(str_buffer->data + str_buffer->len, "%.3lf%n", dis->data, &double_len);
				(str_buffer->len) += double_len;
			}
					                                                                                                                         break;
		case VAR:   BufferPutc (str_buffer, dis_tree->var_names.data[(int) dis->data]);                                                      break;
		case FUNC:
			switch ((int) dis->data)
			{
			case ADD: BufferPutc (str_buffer, '+');                                       break;
			case MUL: BufferPuts (str_buffer, "\\cdot ");                                 break;
			case SUB: BufferPutc (str_buffer, '-');                                       break;
			case SQRT: BufferPuts (str_buffer, "\\sqrt ");                                break;
			case POW: BufferPuts(str_buffer, "^{");                                       break;
			case SIN: BufferPuts (str_buffer, "\\sin ");                                  break;
			case COS: BufferPuts (str_buffer, "\\cos ");                                  break;
			case TAN: BufferPuts (str_buffer, "\\tan ");                                  break;
			case LN:  BufferPuts (str_buffer, "\\ln ");                                   break;
			default: printf ("Unknown func %lf\n", dis->data);                            break;
			}                                                                                                                                 break;
		case CHANGE:
			BufferPutc (str_buffer, 'A' + (int) dis->data);                                                                                   break;
		default: printf ("Unknown type %d\n", dis->type);                                                                                     break;
		}

		if (dis->right && dis->type != CHANGE)
		{
			if (dis->data == POW)
			{
				TeXDumpNode(dis_tree, dis->right, str_buffer);
				BufferPutc(str_buffer, '}');
			}
			else if (dis->right->type == FUNC && dis->data != ADD && dis->data != SUB &&
				    (dis->right->data == ADD || dis->right->data == SUB || TreeSize(dis->right) >= 3))
			{
				BufferPutc (str_buffer, '(');
				TeXDumpNode (dis_tree, dis->right, str_buffer);
				BufferPutc (str_buffer, ')');
			}
			else
				TeXDumpNode (dis_tree, dis->right, str_buffer);
		}
	}
}

void TeXTransform(struct Tree* dis_tree, struct Node* old_node, struct Node* dis, struct Buffer* str_buffer)
{
#include "Puns.h"

	BufferPuts (str_buffer, puns[rand()%PUNS]);

	BufferPuts (str_buffer, "\\begin{center}\r\n"
							"\\\\\r\n"
							"\\\\"
							"${(");
	TeXDumpNode (dis_tree, old_node, str_buffer);
	BufferPuts (str_buffer, ")}^\\prime = ");

	TeXDumpNode (dis_tree, dis, str_buffer);
	BufferPuts (str_buffer, "$\\\\\r\n"
							"\\\\\r\n"
							"\\end{center}");
}

void TeXDumpEnd (struct Buffer* str_buffer, FILE* output, struct Node* dis, struct Tree* dis_tree)
{
	NodeChange (dis_tree->root, &dis_tree->change_buffer);

	BufferPuts (str_buffer, "После некоторого количества преобразований приходим к окончательному и неоспоримому ответу\r\n"
							"\\begin{equation}");

	TeXDumpNode (dis_tree, dis, str_buffer);

	BufferPuts (str_buffer,  "\r\n\\end{equation}\\\\\r\n"
							 "где ");

	for (int i = 0; i < dis_tree->change_buffer.len; i++)
	{
		BufferPutc (str_buffer, 'A' + i);

		BufferPuts (str_buffer,  " = "
								"\r\n\\begin{equation}\r\n");
		
		TeXDumpNode (dis_tree, dis_tree->change_buffer.data[i], str_buffer);
		BufferPuts (str_buffer,  "\r\n\\end{equation}\r\n");
	}

	BufferPuts (str_buffer, "\\section{Вывод}\r\n"
						    "В работе получена формула приозводной заданной функции. Работа выполненна профессионалами и не рекомендуется к выполнению"
						    "слабонервным людям, беременным женщинам и особо невнимательным простофилям. Отдельная благодарность спонсорам работы - "
						    "компании Black Monster Energy\\texttrademark.\r\n"
						    "\\section{Список используемой литературы}\r\n"
						    "\\begin{enumerate}\r\n"
							"\\item Кудрявцев Л Д, К у т а с о в А Д, Чехлов В И, Шабунин М И"
							"Сборник задач по математическому анализу.Том 1 Предел.Не -"
							"прерывность.Дифференцируемость Учеб пособие / Под ред Л Д Кудрявцева — 2 - е изд, перераб ISBN 5 - 9221 - 0306 - 7.\r\n"
							"\\item Великая Бибилия Древних Руссов в переводе Черниковой Л.П.\r\n"
							"\\item Черникова Л.П. Гастроли Ф.И. Шаляпина по Дальнему Востоку оказались роковыми... (монография). Уфа, РИЦ БашГУ, 2008. – 162 с. 12,46 п.л.\r\n"
							"\\item Гитхаб одного красавца \\url{https://github.com/thelimar/Differencator}\r\n"
							"\\end{enumerate}\r\n"
						    "\\end{document}\r\n");

	fwrite (str_buffer->data, sizeof(char), str_buffer->len, output);

	fclose (output);

	BufferDestruct (str_buffer);
}

void NodeChange (struct Node* node, struct NodeBuffer* change_buffer)
{
	int tree_size = 0;
	TreeCount (node, &tree_size);

	if (tree_size <= MAX_OPTIMAL_CHANGE_SIZE && tree_size >= MIN_OPTIMAL_CHANGE_SIZE)
	{
		for (int i = 0; change_buffer->data[i]; i++)
		{
			if (TreesEql (node, change_buffer->data[i]))
			{
				node->data = i;
				node->type = CHANGE;
			}
		}
		if (node->type != CHANGE)
		{
			struct Node* node_copy = NodeCopy (node);
			node->data = change_buffer->len;
			node->type = CHANGE;

			NodeBufferPut (change_buffer, node_copy);
		}
	}
	else
	{
		if (node->left)
			NodeChange (node->left, change_buffer);
		if (node->right)
			NodeChange (node->right, change_buffer);
	}
}

struct Node* GetG (struct Eval* eval, struct Tree* eval_tree)
{
	struct Node* root_node = GetE (eval, eval_tree);
	Require ('$')

	return root_node;
}

struct Node* GetE (struct Eval* eval, struct Tree* eval_tree)
{
	struct Node* add_node = GetT (eval, eval_tree);

	DBG printf ("E %c\n", eval->eval_str[eval->pos]);

	while (eval->eval_str[eval->pos] == '+' || eval->eval_str[eval->pos] == '-')
	{
		if (eval->eval_str[eval->pos] == '+')
		{
			eval->pos++;
			Require (' ')
			add_node = CreateNode (eval_tree, add_node, GetT (eval, eval_tree), ADD, FUNC);
		}
		else if (eval->eval_str[eval->pos] == '-')
		{
			eval->pos++;
			Require (' ')
			add_node = CreateNode (eval_tree, add_node, GetT (eval, eval_tree), SUB, FUNC);
		}
	}

	return add_node;
}

struct Node* GetT (struct Eval* eval, struct Tree* eval_tree)
{
	struct Node* mul_node = GetF (eval, eval_tree);

	DBG printf ("T %c\n", eval->eval_str[eval->pos]);

	while (eval->eval_str[eval->pos] == '*' || eval->eval_str[eval->pos] == '/')
	{
		if (eval->eval_str[eval->pos] == '*')
		{
			eval->pos++;
			Require(' ')
			mul_node = CreateNode (eval_tree, mul_node, GetF (eval, eval_tree), MUL, FUNC);
		}
		else if (eval->eval_str[eval->pos] == '/')
		{
			eval->pos++;
			Require(' ')
			mul_node = CreateNode (eval_tree, mul_node, GetF(eval, eval_tree), DIV, FUNC);
		}
	}

	return mul_node;
}

struct Node* GetF (struct Eval* eval, struct Tree* eval_tree)
{
	struct Node* func_node = GetP(eval, eval_tree);

	char oper[MAX_FUNC_NAME] = "";

	if (eval->eval_str[eval->pos] == '$')
		return func_node;

	DBG printf("F currrent str %s\n", eval->eval_str + eval->pos);

	while (eval->eval_str[eval->pos] == ' ')
		eval->pos++;

	while ((eval->eval_str[eval->pos] < '0' || eval->eval_str[eval->pos] > '9') && eval->eval_str[eval->pos] != '+' 
		    && eval->eval_str[eval->pos] != '-' && eval->eval_str[eval->pos] != ')' && eval->eval_str[eval->pos] != '*' 
		    && eval->eval_str[eval->pos] != '/' && eval->eval_str[eval->pos] != '$')
	{

		DBG printf("F currrent str %s\n", eval->eval_str + eval->pos);

		while (eval->eval_str[eval->pos] == ' ')
			eval->pos++;

		for (int i = 0; (eval->eval_str[eval->pos] < '0' || eval->eval_str[eval->pos] > '9') && eval->eval_str[eval->pos] != ' '; eval->pos++, i++)
		{
			oper[i] = eval->eval_str[eval->pos];
		}

		Require(' ')

		DBG printf ("F oper %s\n", oper);

		if (streql (oper, "^"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), POW, FUNC);
		}
		else if (streql (oper, "sin"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), SIN, FUNC);
		}
		else if (streql (oper, "cos"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), COS, FUNC);
		}
		else if (streql (oper, "tan"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), TAN, FUNC);
		}
		else if (streql (oper, "log"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), LOG, FUNC);
		}
		else if (streql (oper, "ln"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), LN, FUNC);
		}
		else if (streql (oper, "sqrt"))
		{
			func_node = CreateNode (eval_tree, func_node, GetP (eval, eval_tree), SQRT, FUNC);
		}

		for (int i = 0; i < MAX_FUNC_NAME; i++)
		{
			oper[i] = 0;
		}

		while (eval->eval_str[eval->pos] == ' ')
			eval->pos++;
	}

	if (eval->eval_str[eval->pos] == '$' || eval->eval_str[eval->pos] == ')')
		return func_node;

	return func_node;
}

struct Node* GetP (struct Eval* eval, struct Tree* eval_tree)
{
	DBG printf ("P %c\n", eval->eval_str[eval->pos]);

	if (eval->eval_str[eval->pos] == '(')
	{
		eval->pos++;
		DBG printf ("%c\n", eval->eval_str[eval->pos]);
		struct Node* node_out = GetE (eval, eval_tree);
		Require (')')

		return node_out;
	}
	else if (isalpha (eval->eval_str[eval->pos]))
		return GetV (eval, eval_tree);
	else
		return GetN (eval, eval_tree);
}

struct Node* GetN (struct Eval* eval, struct Tree* eval_tree)
{
	DBG printf("N %c\n", eval->eval_str[eval->pos]);

	double val = 0;
	int pos_before = eval->pos;

	while ('0' <= eval->eval_str[eval->pos] && eval->eval_str[eval->pos] <= '9')
	{
		val = val * 10 + (int)eval->eval_str[eval->pos] - '0';
		eval->pos++;
	}

	DBG printf("N %c\n", eval->eval_str[eval->pos]);

	if (pos_before == eval->pos)
	{
		printf("Syntax error in %s\ncurrent str %s\n", __FUNCSIG__, eval->eval_str + eval->pos);
		return NULL;
	}
	else
	{
		printf("return N %lf\n", val);

		return CreateNode (eval_tree, NULL, NULL, val, CONST_T);
	}
}

struct Node* GetV (struct Eval* eval, struct Tree* eval_tree)
{
	if (eval->eval_str[eval->pos] == 's' || eval->eval_str[eval->pos] == 'a' || eval->eval_str[eval->pos] == 'c'
		|| eval->eval_str[eval->pos] == 't' || eval->eval_str[eval->pos] == 'l')
		return NULL;

	if (VarSearch(eval_tree, eval->eval_str[eval->pos]) == -1)
		BufferPutc(&eval_tree->var_names, eval->eval_str[eval->pos]);

	DBG printf("var read %c\n", eval->eval_str[eval->pos]);

	struct Node* var_node = CreateNode(eval_tree, NULL, NULL, VarSearch(eval_tree, eval->eval_str[eval->pos]), VAR);

	eval->pos++;

	return var_node;
}