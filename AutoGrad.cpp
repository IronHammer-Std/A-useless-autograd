/*

The Core Logic Program
Project 2

Input:
A line of Function

Output:
the partial derivative of the functions

ATTENTION:
Use at least C++17 or higher versions
*/

#include <iostream>
#include <numeric>
#include <cmath>
#include <cstring>
#include <cstdint>
/*
Used STL containers
std::vector
std::map
std::string
std::set

Usage:
------------------------std::map------------------------

1. VarMap (std::map<std::string, int>)
- Purpose: Maps variable names to unique integer IDs for efficient 
		   lookup and management.
- Usage: When a new variable is encountered, it is inserted into VarMap with
		 a generated ID. Subsequent lookups use this map to quickly find the 
		 ID associated with a variable name.

2. Factors (std::map<ExprHash, ExprNode**>)
- Purpose: Tracks factors during polynomial simplification to identify common terms.
- Usage: During the simplification process, factors of terms are stored in
		 this map to facilitate combining like terms and extracting common factors.

3. Common (std::map<ExprHash, CommonFactor>)
- Purpose: Identifies common factors between different terms during simplification.
- Usage: When comparing terms to find common factors, this map stores 
		 the relationships between common elements found in different 
		 parts of the expression.

4. Tg (std::map<ExprHash, ExprNode**>)
- Purpose: Temporarily holds hash values of expressions during various 
           simplification steps.
- Usage: In multiple simplification functions, this map is used to 
         track hashes of nodes that have been processed to avoid redundant 
		 operations and ensure each node is simplified efficiently.

std::map is chosen for these scenarios due to its efficient key-value 
storage and retrieval capabilities. It allows for quick lookups, 
insertions, and deletions, which are essential for managing variables, 
tracking expression components during simplification, and optimizing the 
overall performance of the expression processing pipeline.

------------------------std::set------------------------

1. DupStrings (std::set<char*>)
- Purpose: Manages duplicate strings to ensure each unique string is
		   stored only once.
- Usage: When creating string duplicates, it checks if the string already
		 exists in the set to avoid redundancy.

2. Extracted (std::set<ExprHash>)
- Purpose: Keeps track of extracted hash values during the simplification
		   process to prevent reprocessing.
- Usage: During various simplification steps, it stores hashes of
		 processed nodes to ensure each node is simplified only once.

std::set is chosen for these purposes due to its
properties of unique element storage and efficient lookup. It helps
in managing resources efficiently and avoiding unnecessary computations
during the expression tree processing.

------------------------std::vector------------------------

1. Tokens (std::vector<Token>)
- Purpose: Stores the sequence of tokens generated from the input expression.
- Usage: After tokenization, this vector holds all the elements of the 
		 expression, such as numbers, variables, operators, and functions, 
		 which are then used to build the expression tree.

2. Nodes (std::vector<ExprNode*>)
- Purpose: Manages a pool of expression nodes to optimize memory usage 
		   and reuse nodes when possible.
- Usage: Nodes are created and stored in this vector during the construction
		 of the expression tree, allowing for efficient node management and traversal.

3. UnusedNode (std::vector<ExprNode*>)
- Purpose: Keeps track of nodes that are no longer in use and can be 
           reused to avoid memory reallocation.
- Usage: When nodes are released or removed from the tree, they are 
		 added to this vector to be reused later, enhancing performance.

4. Brackets (std::vector<BracketPtr>)
- Purpose: Tracks the positions of brackets during the parsing process 
           to handle nested expressions correctly.
- Usage: As the parser encounters brackets, it records their positions 
		 in this vector to manage the scope and priority of operations 
		 within brackets.

5. TraverseSeries (std::vector<ExprNode*>)
- Purpose: Temporarily stores nodes during tree traversal operations.
- Usage: Various simplification and manipulation functions use this 
		 vector to collect nodes of specific types or properties 
		 for further processing.

6. ExprV (std::vector<ExprNode*>)
- Purpose: Used during the creation of subtrees to hold 
		   intermediate expression nodes.
- Usage: When constructing complex expressions, this vector helps 
		 manage the nodes that form parts of the larger expression tree.

7. OprV (std::vector<ExprNode*>)
- Purpose: Stores operator nodes during the parsing and tree construction process.
- Usage: Helps in managing the order and precedence of operators 
		 when building the expression tree from the token sequence.

std::vector is selected for these purposes due to its dynamic size 
management and efficient element access. It provides a flexible way to 
store and manipulate sequences of elements, making it suitable for managing 
tokens, nodes, and other components during the expression processing pipeline.

------------------------std::string------------------------

1. Vars (std::vector<std::string>)
- Purpose: Stores the names of variables encountered in the expression.
- Usage: When a new variable is found during tokenization, its name 
		 is stored here for reference and management.

2. Expression (std::string)
- Purpose: Holds the input mathematical expression as a string.
- Usage: This string is processed and parsed to generate tokens 
		 and build the expression tree.

3. Token::GetText() method
- Purpose: Returns the textual representation of a token.
- Usage: Depending on the token type, it constructs and returns a 
		 string that represents the token's value.

4. HashStr functions
- Purpose: Computes a hash value for a given string.
- Usage: These functions take a string (like variable or function names) 
		 and compute a hash to be used in various hash-based operations.

5. DuplicateStr and related functions
- Purpose: Creates duplicate strings with proper memory management.
- Usage: Ensures that strings used in the expression tree are properly 
		 allocated and managed to avoid memory leaks.

6. Function names in the Funcs array
- Purpose: Stores the names of mathematical functions.
- Usage: These strings are used to identify and match functions during 
		 the parsing and evaluation of the expression tree.

std::string is essential for handling textual data in the code. It provides 
flexible string manipulation capabilities, allowing for efficient storage and 
processing of variable names, function names, and the input expression itself. 
This makes it possible to correctly parse and interpret the mathematical 
expressions provided by the user.
*/
#include <string>
#include <set>
#include <map>
#include <vector>

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-------------------------DEBUG CONSTANTS-------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

//Switch data parsing debug output on/off
const bool EnableDebugData = false;

//Switch data simplify debug output on/off
const bool EnableDebugSimplifyI = false;

//Switch data simplify debug output on/off
const bool EnableDebugSimplifyII = false;



//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-------------------------TOOL FUNCTIONS--------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------



//the global Hash Type
using ExprHash = unsigned long long;
/**
 * Transforms a hash value to reduce collisions and improve distribution.
 * Distribute the Hash value to [0,2^64)
 *
 * @param H The input hash value to be transformed.
 * @return The transformed hash value.
 */
ExprHash TransformHash(ExprHash H)
{
	/*
	We can imply that for Linear Congruential Transformation,
	Result = H * A + C, result and H are in [0, M) (M=2^64)
	we need to ensure that:
	gcd(C, M)=1
	A mod 8 = 1
	this multiplier A = 6364136223846793005 because STL selects it
	*/
	return H * 6364136223846793005 + 7;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//---------FUNDAMENTAL TYPE DEFINITION & GLOBAL VARIABLES----------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/*
- Purpose: Manages duplicate strings to ensure each unique string is
		   stored only once.
- Usage: When creating string duplicates, it checks if the string already
		 exists in the set to avoid redundancy.
*/
std::set<char*> DupStrings;

/**
 * @brief Duplicates a string and manages it within a set to prevent duplicates.
 *
 * @param V The string to be duplicated.
 * @param Len The length of the string to be duplicated.
 * @return char* A pointer to the duplicated string stored in the set.
 */
char* DuplicateStr(const char* V, size_t Len)
{
	auto Iter = DupStrings.insert(new char[Len + 1] {}).first;
	strncpy(*Iter, V, Len);
	return *Iter;
}
/**
 * @brief Creates a new string of a specified length and manages it within a set.
 *
 * @param Len The desired length of the new string.
 * @return char* A pointer to the newly created string stored in the set.
 */
char* MakeStr(size_t Len)
{
	auto Iter = DupStrings.insert(new char[Len + 1] {}).first;
	return *Iter;
}
int StrToIntEx(const char* Begin, const char* End);

/*
- Purpose: Records the amount of used variables
- Usage: When a new variable is found during tokenization, VarMaxID increases by 1.
*/
int VarMaxID{ 0 };
/*
- Purpose: Stores the names of variables encountered in the expression.
- Usage: When a new variable is found during tokenization, its name 
		 is stored here for reference and management.
*/
std::vector<std::string> Vars;
/*
- Purpose: Maps variable names to unique integer IDs for efficient
		   lookup and management.
- Usage: When a new variable is encountered, it is inserted into VarMap with
		 a generated ID. Subsequent lookups use this map to quickly find the
		 ID associated with a variable name.
*/
std::map<std::string, int> VarMap;
int GetVarID(const char* Name);

/**
 * @brief A placeholder struct used to explicitly mark function IDs.
 *
 * This struct serves as a tag to distinguish function IDs from other integer values
 * when constructing tokens, ensuring type safety and clarity in the code.
 */
struct AsFuncID final {};

/**
 * @brief Represents a token in the mathematical expression, such as a number, variable, function, or operator.
 */
struct Token
{
	//Enumerates the possible types of tokens.
	enum Type
	{
		Int,
		Variable,
		Function,
		Operator
	};

	//Members
	///< The type of the token.
	Type Ty{ Int };
	///< The identifier or value of the token.
	int ID{ 0 };

	//Member Functions
	Token() = default;
	Token(int V);
	Token(char Opr);
	Token(int FuncID, AsFuncID);
	Token(char* Var);
	const char* GetText() const;
	bool IsLBK() const { return Ty == Operator && ID == '('; }//equals to '('
	bool IsRBK() const { return Ty == Operator && ID == ')'; }//equals to '('
	bool IsCOM() const { return Ty == Operator && ID == ','; }//equals to ','
	bool IsPOW() const;
	bool IsIgnoredSymbol() const { return Ty == Operator && (ID == '(' || ID == ')' || ID == ','); }
	bool IsSUB() const { return Ty == Operator && ID == '-'; }//equals to ','
	/**
	 * @brief Compares this token with another for equality.
	 *
	 * @param R The token to compare with.
	 * @return true If the tokens are equal.
	 * @return false Otherwise.
	 */
	bool operator==(const Token& R) const { return Ty == R.Ty && ID == R.ID; }
	/**
	 * Note : sizeof(Token)==sizeof(ExprHash) so this is safe
	 * @brief Computes a hash value for the token.
	 * 
	 * @return ExprHash The hash value of the token.
	 */
	ExprHash Hash() const { return TransformHash(*reinterpret_cast<const ExprHash*>(this)); }
};

/*
- Purpose: Stores the sequence of tokens generated from the input expression.
- Usage: After tokenization, this vector holds all the elements of the
		 expression, such as numbers, variables, operators, and functions,
		 which are then used to build the expression tree.
*/
std::vector<Token> Tokens;

//Declaration of Derivative functions
struct ExprNode;
ExprNode* DX_ln(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_log(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_cos(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_sin(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_tan(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_pow(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_exp(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_sinh(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_cosh(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_add(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_sub(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_mul(const ExprNode* Op1, const ExprNode* Op2, int DX);
ExprNode* DX_div(const ExprNode* Op1, const ExprNode* Op2, int DX);

/**
 * @brief Represents a mathematical expression and provides 
 functionality for parsing, differentiation, and simplification.
 */
struct Expr
{
	//Members
	///< The root node of the expression tree.
	ExprNode* Root{ nullptr };

	//Member Functions
	Expr() = delete;
	~Expr();
	/**
	 * @brief Constructs an expression from a sequence of tokens.
	 *
	 * @param Toks The vector of tokens representing the mathematical expression.
	 */
	Expr(const std::vector<Token>& Toks);
	/**
	 * @brief Constructs an expression by differentiating another expression with respect to a variable.
	 *
	 * @param F The original expression to differentiate.
	 * @param DX The ID of the variable with respect to which to differentiate.
	 */
	Expr(const Expr& F, int DX);
	/**
	 * @brief Prints the expression to the standard output.
	 */
	void Print() const;
};

/**
 * @brief Represents a mathematical function with its properties and derivative rules.
 *
 * This class encapsulates the necessary information for handling mathematical functions
 * within the expression tree, including the function's name, the number of parameters it expects,
 * and a pointer to the function that computes its derivative.
 */
struct Function
{
	//Members
	///< The name of the function (e.g., "sin", "cos").
	const char* Name;
	///< The number of parameters the function takes.
	int NParam; 
	///< Pointer to the derivative function.
	ExprNode* (*Derivative)(const ExprNode* Op1, const ExprNode* Op2, int DX);
};

/*
- Purpose: Maps function names to their respective metadata, including the
           number of parameters and derivative rules.
- Usage: During expression parsing, when a function token is encountered, 
         the parser looks up this array to validate the function and retrieve 
		 its details. It is also used during differentiation to apply the 
		 correct derivative rules.
 */
Function Funcs[] = {
#define FUNC_ln 0
	{"ln",1,DX_ln},
#define FUNC_log 1
	{"log",2,DX_log},
#define FUNC_cos 2
	{"cos",1,DX_cos},
#define FUNC_sin 3
	{"sin",1,DX_sin},
#define FUNC_tan 4
	{"tan",1,DX_tan},
#define FUNC_pow 5
	{"pow",2,DX_pow},
#define FUNC_exp 6
	{"exp",1,DX_exp},
//Added EXT
#define FUNC_sinh 7
	{"sinh",1,DX_sinh},
//Added EXT
#define FUNC_cosh 8
	{"cosh",1,DX_cosh}
};
//Count of the Functions
const int NFuncs = sizeof(Funcs) / sizeof(Function);
int GetFuncID(const char* Name);//if not found return -1

/*
An array containing all valid operators and symbols used in mathematical expressions.
- Purpose: To provide a centralized list of characters recognized as 
           operators or special symbols.
- Usage: This array is referenced during tokenization to identify 
         operators and symbols in the input string.
*/
char Operators[] = "+-*/^,()";
const int NOpr = sizeof(Operators) - 1;

/**
 * @brief Checks if a character is a valid operator or special symbol.
 *
 * @param c The character to check.
 * @return true If the character is found in the Operators array.
 * @return false Otherwise.
 */
bool IsOpr(char c)
{
	for (int i = 0; i < NOpr; i++)
		if (c == Operators[i])
			return true;
	return false;
}

/**
 * @brief Represents a node in the expression tree, containing information about the token it holds and its child nodes.
 */
struct ExprNode
{
	///< The token stored in this node (e.g., integer, variable, function, or operator).
	Token V{};
	///< Pointer to the previous node in the expression tree.
	ExprNode* Prev{};
	///< Pointer to the next node in the expression tree.
	ExprNode* Next{};
	///< Array storing pointers to child nodes (left and right operands).
	ExprNode* Operand[2]{};

	/**
	 * @brief Returns the operator precedence level of this node.
	 *
	 * @param ConsiderUsedNode Whether to consider if the node is already used.
	 * @return int The precedence level of the operator.
	 */
	int OprLevel(bool ConsiderUsedNode) const;

	/**
	 * @brief Prints the node's information for debugging purposes.
	 */
	void DebugPrint() const;
	void DebugPrintTraverse(const ExprNode* End) const;

	/**
	 * @brief Prints the subtree rooted at this node in a tree-like structure.
	 *
	 * @param Parent The parent node of this node.
	 * @param PrintedCount A counter for the number of nodes printed.
	 * @param IsLeft Indicates if this node is the left child of its parent.
	 */
	void PrintTree(const ExprNode* Parent, int& PrintedCount, bool IsLeft) const;
	void PrintTree(const ExprNode* Parent = nullptr) const { int C{}; PrintTree(Parent, C, false); }

	/**
	 * @brief Computes the partial derivative of this node with respect to a variable.
	 *
	 * @param DX The ID of the variable with respect to which to differentiate.
	 * @return ExprNode* The resulting node after differentiation.
	 */
	ExprNode* Partial(int DX) const;

	/**
	 * @brief Computes the partial derivative for operator nodes.
	 *
	 * @param DX The ID of the variable with respect to which to differentiate.
	 * @return ExprNode* The resulting node after differentiation.
	 */
	ExprNode* PartialOpr(int DX) const;

	/**
	 * @brief Creates a copy of this node.
	 *
	 * @return ExprNode* A duplicate of this node.
	 */
	ExprNode* Duplicate() const;

	/**
	 * @brief Computes the hash value of this node.
	 *
	 * @return ExprHash The hash value of the node.
	 */
	ExprHash Hash() const;

	/**
	 * @brief Checks if this node represents a squared term.
	 *
	 * @return bool True if the node is a squared term, false otherwise.
	 */
	bool IsSquare() const;

	/**
	 * @brief Checks if this node represents a constant value.
	 *
	 * @return bool True if the node is a constant, false otherwise.
	 */
	bool IsConst() const;

	/**
	 * @brief Checks if this node represents a constant value excluding radicals.
	 *
	 * @return bool True if the node is a constant in the specified form, false otherwise.
	 */
	bool IsConstII() const;

	/**
	 * @brief Checks if the left operand exists.
	 *
	 * @return bool True if the left operand exists, false otherwise.
	 */
	bool HasOp0() const { return Operand[0] != nullptr; }

	/**
	 * @brief Checks if the right operand exists.
	 *
	 * @return bool True if the right operand exists, false otherwise.
	 */
	bool HasOp1() const { return Operand[1] != nullptr; }

	/**
	 * @brief Gets the type of the left operand's token.
	 *
	 * @return Token::Type The type of the left operand's token.
	 */
	Token::Type Ty0() const { return Operand[0]->V.Ty; }

	/**
	 * @brief Gets the type of the right operand's token.
	 *
	 * @return Token::Type The type of the right operand's token.
	 */
	Token::Type Ty1() const { return Operand[1]->V.Ty; }

	/**
	 * @brief Gets the ID of the left operand's token.
	 *
	 * @return int& The ID of the left operand's token.
	 */
	int& ID0() const { return Operand[0]->V.ID; }

	/**
	 * @brief Gets the ID of the right operand's token.
	 *
	 * @return int& The ID of the right operand's token.
	 */
	int& ID1() const { return Operand[1]->V.ID; }

	/**
	 * @brief Gets the left operand's token.
	 *
	 * @return Token: The left operand's token.
	 */
	Token V0() const { return Operand[0]->V; }

	/**
	 * @brief Gets the right operand's token.
	 *
	 * @return Token: The right operand's token.
	 */
	Token V1() const { return Operand[1]->V; }

	/**
	* @brief Gets the left operand node.
	*
	* @return ExprNode*& The left operand node.
	*/
	ExprNode*& L() { return Operand[0]; }

	/**
	 * @brief Gets the right operand node.
	 *
	 * @return ExprNode*& The right operand node.
	 */
	ExprNode*& R() { return Operand[1]; }

	/**
	 * @brief Gets the left operand node (const version).
	 *
	 * @return const ExprNode* The left operand node.
	 */
	const ExprNode* L() const { return Operand[0]; }

	/**
	 * @brief Gets the right operand node (const version).
	 *
	 * @return const ExprNode* The right operand node.
	 */
	const ExprNode* R() const { return Operand[1]; }

	/**
	 * @brief Copy data from another node.
	 */
	void Copy(const ExprNode* r)
	{
		V = r->V;
		Operand[0] = r->Operand[0];
		Operand[1] = r->Operand[1];
	}
};

/*
- Purpose: Manages a pool of expression nodes to optimize memory usage
		   and reuse nodes when possible.
- Usage: Nodes are created and stored in this vector during the construction
		 of the expression tree, allowing for efficient node management and traversal.
*/
std::vector<ExprNode*> Nodes;
/*
- Purpose: Keeps track of nodes that are no longer in use and can be
		   reused to avoid memory reallocation.
- Usage: When nodes are released or removed from the tree, they are
		 added to this vector to be reused later, enhancing performance.
*/
std::vector<ExprNode*> UnusedNode;

/**
 * @brief A structure used to track bracket positions during expression parsing.
 */
struct BracketPtr
{
	ExprNode* Left;///< Pointer to the left bracket node.
	ExprNode* Comma;///< Pointer to the comma node within the brackets.
};

/*
- Purpose: Tracks the positions of brackets during the parsing process
		   to handle nested expressions correctly.
- Usage: As the parser encounters brackets, it records their positions
		 in this vector to manage the scope and priority of operations
		 within brackets.
*/
std::vector <BracketPtr> Brackets;

/*
- Purpose: Used during the creation of subtrees to hold
		   intermediate expression nodes.
- Usage: When constructing complex expressions, this vector helps
		 manage the nodes that form parts of the larger expression tree.
*/
std::vector <ExprNode*> ExprV;

/*
- Purpose: Stores operator nodes during the parsing and tree construction process.
- Usage: Helps in managing the order and precedence of operators
		 when building the expression tree from the token sequence.
*/
std::vector <ExprNode*> OprV;

/*
- Purpose: Keeps track of extracted hash values during the simplification
		   process to prevent reprocessing.
- Usage: During various simplification steps, it stores hashes of
		 processed nodes to ensure each node is simplified only once.
*/
std::set<ExprHash> Extracted;

/*
A flag indicating whether the parsing process has failed.
- Purpose: To signal errors during the parsing of the mathematical expression.
- Usage: Checked after parsing to determine if the expression was valid.
*/
bool FailedToParse;

/*
A flag indicating whether there is a DividedbyZero occurred
- Purpose: To signal errors during the calculating of the mathematical expression.
- Usage: Checked after calculating to determine if the expression was valid.
*/
bool DividedbyZero;

/**
 * @brief Releases a node from the expression tree, marking it as unused.
 *
 * @param pNode The node to be released.
 */
void ReleaseNode(const ExprNode*);

/**
 * @brief Simplifies the expression tree rooted at the given node.
 *
 * @param pNode The root node of the subtree to be simplified.
 */
void Simplify(ExprNode*& pNode);

/**
 * @brief Creates a new node for the expression tree.
 *
 * @return ExprNode* A pointer to the newly created node.
 */
ExprNode* CreateNode();
ExprNode* CreateNode(Token T);
ExprNode* CreateNode(Token T, ExprNode* Op1, ExprNode* Op2);

/**
 * @brief Releases the memory allocated for an expression tree.
 *
 * @param pRoot The root node of the expression tree to be released.
 */
void ReleaseTree(const ExprNode* pRoot);

/**
 * @brief Generates tokens from a mathematical expression string.
 *
 * @param Str The input string representing the mathematical expression.
 * @param Toks A reference to a vector where the generated tokens will be stored.
 */
void GenerateTokens(const std::string& Str, std::vector<Token>& Toks);

/*
- Purpose: Holds the input mathematical expression as a string.
- Usage: This string is processed and parsed to generate tokens
		 and build the expression tree.
*/
std::string Expression;

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//--------------MULTI TEST CASE RESOURCE CLEANER-------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief A guard class to manage resources and reset global states for each parsing round.
 */
struct RoundGuard
{
	RoundGuard();
	~RoundGuard();
};

/**
* @brief Initializes a new round by resetting global states.
*
* Clears variable mappings, extracted hashes, and other relevant data structures
* to ensure a clean state for the upcoming parsing and processing.
*/
RoundGuard::RoundGuard()
{
	VarMap.clear();
	Vars.clear();
	Extracted.clear();
}

/**
* @brief Cleans up allocated resources and resets global states.
*
* Releases memory for duplicated strings and nodes, clears all global containers,
* and resets counters and flags to their default states.
*/
RoundGuard::~RoundGuard()
{
	for (auto& p : DupStrings)delete[] p;
	DupStrings.clear();
	for (auto& p : Nodes)delete p;
	Nodes.clear();
	UnusedNode.clear();
	Tokens.clear();
	VarMaxID = 0;
	FailedToParse = false;
	DividedbyZero = false;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//------------------------EXPRESSION PARSER------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief Enumerates the possible token types when parsing characters in an expression.
 */
enum class TokCond {
	Null,       ///< No valid token type.
	Operator,   ///< Operator token (e.g., +, -, *, /).
	Symbol,     ///< Symbol token (e.g., variable names, function names).
	Number      ///< Number token.
};

/**
 * @brief Determines the token type of a given character.
 *
 * @param c The character to be judged.
 * @return TokCond The token type of the character.
 */
TokCond Judge(char c)
{
	if (isdigit(c))return TokCond::Number;
	else if (isalpha(c))return TokCond::Symbol;
	else if (IsOpr(c))return TokCond::Operator;
	//Note : ignore invalid characters 
	else return TokCond::Null;
}
/**
 * @brief Parses a sequence of characters into a token based on the specified type.
 *
 * @param Type The type of the token to parse.
 * @param Begin The start of the character sequence.
 * @param Idx The end of the character sequence.
 * @param Toks The vector to which the parsed token will be added.
 */
void ParseToken(TokCond Type, const char* Begin, const char* Idx, std::vector<Token>& Toks)
{
	switch (Type)
	{
		// Nothing to do for null type
	case TokCond::Null: break;
		// Create operator token from single character
	case TokCond::Operator: Toks.emplace_back(*Begin); break;
		// Convert numeric string to integer token
	case TokCond::Number: Toks.emplace_back(StrToIntEx(Begin, Idx)); break;
		// Handle symbols which could be variables or functions
	case TokCond::Symbol:
	{
		// Duplicate the string for symbol
		char* p = DuplicateStr(Begin, Idx - Begin);
		// Check if it's a known function
		int ID = GetFuncID(p);
		// If not a function, create variable token
		if (ID == -1)Toks.emplace_back(DuplicateStr(Begin, Idx - Begin));
		// If it is a function, create function token
		else Toks.emplace_back(ID, AsFuncID{});
	}break;
	};
}

/**
 * @brief Generates tokens from a mathematical expression string.
 *
 * @param Str The input string representing the mathematical expression.
 * @param Toks A reference to a vector where the generated tokens will be stored.
 */
void GenerateTokens(const std::string& Str, std::vector<Token>& Toks)
{
	auto Begin = Str.begin(), Idx = Str.begin();
	TokCond LastType{ TokCond::Null }, CurType;
	for (; Idx != Str.end(); ++Idx)
	{
		// Determine current character type
		CurType = Judge(*Idx);
		if (LastType == CurType)
		{
			// Handle consecutive characters of same type
			if (CurType == TokCond::Operator)
			{
				// Operators are single-character, so add immediately
				Toks.emplace_back(*Begin);
				Begin = Idx;
			}
			continue;
		}
		// Parse previous token segment
		ParseToken(LastType, Begin.operator->(), Idx.operator->(), Toks);

		// Update for next iteration
		Begin = Idx;
		LastType = CurType;
	}
	// Parse the last token segment
	ParseToken(LastType, Begin.operator->(), Idx.operator->(), Toks);
}

//ADD COMMENTS FROM HERE !!!

// Token constructor implementations

/**
 * @brief Constructs a Token from an integer value.
 * @param V The integer value to store in the token.
 */
Token::Token(int V) : Ty(Token::Type::Int), ID(V) {}

/**
 * @brief Constructs a Token from an operator character.
 * @param Opr The operator character (e.g., '+', '-', '*', etc.).
 */
Token::Token(char Opr) : Ty(Token::Type::Operator), ID(Opr) {}

/**
 * @brief Constructs a Token representing a function with a given ID.
 * @param FuncID The ID of the function as defined in Funcs array.
 * @param (Unused) A tag to differentiate this constructor from others.
 */
Token::Token(int FuncID, AsFuncID) : Ty(Token::Type::Function), ID(FuncID) {}

/**
 * @brief Constructs a Token representing a variable.
 * @param Var The name of the variable. Automatically assigns a unique ID via GetVarID.
 */
Token::Token(char* Var) : Ty(Token::Type::Variable), ID(GetVarID(Var)) {}

/**
 * @brief Returns the textual representation of the token based on its type.
 * @return const char* The string representation (function name, operator char, variable name, or integer value).
 */
const char* Token::GetText() const
{
	switch (Ty)
	{
	case Token::Function: return Funcs[ID].Name;
		// Lookup function name from Funcs array
	case Token::Operator:
	{
		// Convert operator ID to a string
		char* p = MakeStr(1);
		p[0] = (char)ID;
		return p;
	}
	// Get variable name from Vars vector
	case Token::Variable: return Vars[ID].c_str();
		// Convert integer ID to string
	case Token::Int:
	{
		char* p = MakeStr(16);
		snprintf(p, 16, "%d", ID);
		return p;
	}
	default: return "";//solve a warning
	}
}

/**
 * @brief Converts a substring to an integer.
 * @param Begin Start iterator of the substring.
 * @param End End iterator of the substring.
 * @return int The parsed integer value.
 */
int StrToIntEx(const char* Begin, const char* End)
{
	int Value = 0;// Only handles non-negative integers
	while (Begin < End)
	{
		Value *= 10;
		Value += *Begin - '0';
		++Begin;
	}
	return Value;
}

/**
 * @brief Looks up a function ID by name.
 * @param Name The function name to search for.
 * @return int The function ID if found, -1 otherwise.
 */
int GetFuncID(const char* Name)
{
	for (int i = 0; i < NFuncs; i++)
	{
		if (!strcmp(Funcs[i].Name, Name))
			return i;
	}
	return -1;
}

/**
 * @brief Retrieves or creates a variable ID for a given variable name.
 * @param Name The variable name.
 * @return int Unique integer ID assigned to the variable.
 */
int GetVarID(const char* Name)
{
	auto it = VarMap.find(Name);
	if (it != VarMap.end())return it->second;
	else
	{
		VarMap[Name] = VarMaxID;
		Vars.push_back(Name);
		return VarMaxID++;
	}
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//------------------EXPRESSION TREE CONSTRUCTION-------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief Determines if operator merging should occur based on precedence.
 * @param Back The previous operator node.
 * @param CurLevel The precedence level of the current operator.
 * @return bool True if merging should trigger.
 */
bool TriggerMerge(const ExprNode* Back, int CurLevel)
{
	// Handle right-associative operators like '-' and '/'
	if (Back->V.Ty == Token::Operator && (Back->V.ID == '-' || Back->V.ID == '/'))
		return Back->OprLevel(false) >= CurLevel;
	else return Back->OprLevel(false) > CurLevel;
}

/**
 * @brief Merges operator nodes in the operator stack based on precedence.
 * @param CurLevel The precedence level of the current operator.
 * @return bool False if syntax error occurs (e.g., missing operands).
 */
bool MergeTop(int CurLevel)
{
	while (!OprV.empty() && TriggerMerge(OprV.back(), CurLevel))
	{
		if (ExprV.size() < 2) { puts("Syntax Error: missing operand."); FailedToParse = true; return false; }
		
		// Build tree: attach operands to operator
		OprV.back()->R() = ExprV.back();
		OprV.back()->L() = ExprV[ExprV.size() - 2];
		ExprV[ExprV.size() - 2] = OprV.back();
		OprV.pop_back();
		ExprV.pop_back();
	}
	return true;
}

/**
 * @brief Constructs an expression tree from a token sequence.
 * @param Begin Start node of the token sequence.
 * @param End End node of the token sequence (exclusive).
 * @return ExprNode* Root of the constructed subtree.
 */
ExprNode* CreateTree(ExprNode* Begin, ExprNode* End)// PARSE TO: [Begin,End)
{
	extern Token MUL;
	if constexpr (EnableDebugData) { printf("\nEnd: "); End->DebugPrint(); printf("\nToCreate: "); Begin->DebugPrintTraverse(End); }
	//Note: Prev always exists
	if (Begin == End) return CreateNode();

	// Handle unary minus: prepend a zero (e.g., "-x" -> "0 - x")
	else if (Begin->OprLevel(false) && Begin->V.IsSUB())
	{
		auto V = CreateNode(Token((int)0));
		V->Next = Begin;
		V->Prev = Begin->Prev;
		Begin->Prev->Next = V;
		Begin->Prev = V;
		Begin = V;
	}

	OprV.clear();
	ExprV.clear();
	int Level;
	bool LastHasLevel = true;
	auto UP = Begin->Prev;

	// Process each node in the token sequence
	for (auto pNode = Begin; pNode != End; pNode = pNode->Next)
	{
		// Skip brackets/comma
		if (pNode->V.IsIgnoredSymbol())continue;
		Level = pNode->OprLevel(false);

		// Operand (number/variable)
		if (!Level)
		{
			// Implicit multiplication (e.g., "2x")
			if (!LastHasLevel)
			{
				if (!MergeTop(2))return CreateNode();
				OprV.push_back(CreateNode(MUL));
			}
			ExprV.push_back(pNode);
		}
		// Operator or function
		else
		{
			if (!MergeTop(Level))return CreateNode();
			OprV.push_back(pNode);
		}
		LastHasLevel = Level != 0;
		if constexpr (EnableDebugData) { printf("\nPush: "); pNode->DebugPrint(); }
	}
	// Merge remaining operators
	while (!OprV.empty())if (!MergeTop(0))return CreateNode();

	// Link the final tree
	ExprV[0]->Next = End;
	ExprV[0]->Prev = UP;
	End->Prev = ExprV[0];
	UP->Next = ExprV[0];

	if constexpr (EnableDebugData) { printf("\nCreate: "); ExprV[0]->DebugPrint(); }

	return ExprV[0];
}

/**
 * @brief Validates function argument counts in the expression tree.
 * @param E Root node of the subtree to check.
 * @return bool True if all functions have correct parameter counts.
 */
bool CheckArgument(const ExprNode* E)
{
	if (!E)return true;
	else if (E->V.Ty == Token::Function)
	{
		int NArg = !!(E->L()) + !!(E->R());
		if (NArg != Funcs[E->V.ID].NParam)
		{
			printf("Syntax Error: Function %s expected %d Arguments, Found %d Arguments\n", Funcs[E->V.ID].Name, Funcs[E->V.ID].NParam, NArg);
			return false;
		}
		else return true;
	}
	// Recursively check children
	else
	{
		bool OK = true;
		OK &= CheckArgument(E->L());
		OK &= CheckArgument(E->R());
		return OK;
	}
}

/**
 * @brief Destructor for Expr. Releases the entire expression tree.
 */
Expr::~Expr()
{
	ReleaseTree(Root);
}


Expr::Expr(const std::vector<Token>& Toks)
{
	DividedbyZero = false;
	ExprNode* pNew;
	ExprNode M{}, N{};
	ExprNode* pCur = &M;
	Brackets.clear();

	// Build linked list of tokens
	for (auto& T : Toks)
	{
		pNew = CreateNode(T);
		pCur->Next = pNew;
		pNew->Prev = pCur;
		pCur = pNew;
		if constexpr (EnableDebugData) { putchar('\n'); printf("New: "); M.Next->DebugPrintTraverse(nullptr); }

		// Handle brackets and commas for function arguments
		if (T.IsLBK())
		{
			Brackets.emplace_back();
			Brackets.back().Left = pCur;
			Brackets.back().Comma = nullptr;
			if constexpr (EnableDebugData) { puts("Push:"); pCur->DebugPrint(); }
		}
		else if (T.IsCOM())
		{
			if (!Brackets.empty())
			{
				if (Brackets.back().Comma) { puts("Syntax Error: Too many arguments."); FailedToParse = true; return; }
				else Brackets.back().Comma = pCur;
			}
			else { puts("Syntax Error: \",\"is not in a \"()\"."); FailedToParse = true; return; }
		}
		else if (T.IsRBK())
		{
			if (Brackets.empty()) { puts("Syntax Error: \")\"is lonely."); FailedToParse = true; return; }
			if (Brackets.back().Comma)
			{
				auto pTok = Brackets.back().Left->Prev;
				auto pCom = Brackets.back().Comma;
				//putchar('\n'); M.Next->DebugPrintTraverse(nullptr);
				//putchar('\n'); M.Next->DebugPrintTraverse(Brackets.back().Left);
				auto T1 = CreateTree(Brackets.back().Left->Next, pCom);
				auto T2 = CreateTree(pCom->Next, pCur);
				if (FailedToParse)return;
				//pTok-> ( ->T1->Comma->T2->pCur
				if (pTok->V.Ty == Token::Function)
				{
					ReleaseNode(pCur);//)
					pTok->L() = T1;
					pTok->R() = T2;
					pCur = pTok;
				}
				else { puts("Syntax Error: \",\" is only for functions."); FailedToParse = true; return; }
			}
			// Single argument in brackets
			else
			{
				auto pTok = Brackets.back().Left->Prev;
				auto T1 = CreateTree(Brackets.back().Left->Next, pCur);
				Brackets.back().Left->Next = T1;
				T1->Next = pCur;
				if (FailedToParse)return;
				if constexpr (EnableDebugData) { putchar('\n'); M.Next->DebugPrintTraverse(nullptr); }
				//L->T->pCur
				//ReleaseNode(pCur);//)
				if (pTok->V.Ty == Token::Function)
				{
					pTok->L() = T1;
					pCur = pTok;
					pCur->Next = nullptr;
					//if (EnableDebugData) { putchar('\n'); M.Next->DebugPrintTraverse(nullptr); }
				}
				else
				{
					pTok->Next = T1;
					T1->Prev = pTok;
					pCur = T1;
				}
			}
			Brackets.pop_back();
		}
	}
	if (!Brackets.empty()) { puts("Syntax Error: expected \")\"for a lonely \"(\" qwq. "); FailedToParse = true; return; }
	
	// Finalize tree construction
	pCur->Next = &N;
	N.Prev = pCur;
	if constexpr (EnableDebugData) puts("Make End!");
	Root = CreateTree(M.Next, &N);

	// Check Arguments
	if (!CheckArgument(Root)) { FailedToParse = true; return; }

	// Apply simplification rules
	Simplify(Root);
}

/**
 * @brief Prints the expression tree.
 */
void Expr::Print() const
{
	if (!DividedbyZero)
	{
		if (Root)Root->PrintTree();
		else printf("NULL");
		putchar('\n');
	}
}

/**
 * @brief Constructs an Expr by differentiating another Expr.
 * @param F The original expression to differentiate.
 * @param DX The variable ID to differentiate with respect to.
 */
Expr::Expr(const Expr& F, int DX)
{
	DividedbyZero = false;
	Root = F.Root->Partial(DX);
	Simplify(Root);
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------NODE MEMORY MANAGER-----------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief Releases a node from the expression tree, marking it as unused.
 *
 * @param pNode The node to be released.
 */
void ReleaseNode(const ExprNode* pNode)
{
	if constexpr (EnableDebugData) { printf("\nReleaseNode:"); if (pNode)pNode->DebugPrint(); else printf("NULL"); }
	if (pNode)UnusedNode.push_back((ExprNode*)pNode);
}

/**
 * @brief Resets a node's state for reuse.
 * @param pNode The node to clear.
 * @return ExprNode* The cleared node.
 */
ExprNode* ClearNode(ExprNode* pNode)
{
	pNode->L() = nullptr;
	pNode->R() = nullptr;
	pNode->V = Token();
	pNode->Next = nullptr;
	pNode->Prev = nullptr;
	return pNode;
}

/**
 * @brief Creates a new node for the expression tree.
 *
 * @return ExprNode* A pointer to the newly created node.
 */
ExprNode* CreateNode()
{
	if (UnusedNode.empty()) { Nodes.push_back(new ExprNode); return Nodes.back(); }
	else { auto P = ClearNode(UnusedNode.back()); UnusedNode.pop_back(); return P; }
}

/**
 * @brief Creates a node initialized with a token.
 * @param T The token to store in the node.
 * @return ExprNode* The created node.
 */
ExprNode* CreateNode(Token T)
{
	auto N = CreateNode();
	N->V = T;
	return N;
}

/**
 * @brief Creates a node with operands.
 * @param T The token for the node.
 * @param Op1 Left operand.
 * @param Op2 Right operand.
 * @return ExprNode* The created node.
 */
ExprNode* CreateNode(Token T, ExprNode* Op1, ExprNode* Op2)
{
	auto N = CreateNode();
	N->V = T;
	N->L() = Op1;
	N->R() = Op2;
	return N;
}

/**
 * @brief Recursively releases an entire subtree.
 * @param pRoot Root of the subtree to release.
 */
void ReleaseTree(const ExprNode* pRoot)
{
	if (pRoot)
	{
		ReleaseTree(pRoot->L());
		ReleaseTree(pRoot->R());
		ReleaseNode(pRoot);
	}
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//---------------------TREE GENERATION HELPERS---------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

bool Token::IsPOW() const { return (Ty == Operator && ID == '^') || (Ty == Function && ID == FUNC_pow); }

/**
 * @brief Helper structure for function token generation and tree construction.
 * @tparam T The token type to handle (functions/operators)
 */
struct NodeCalcHelper
{
	Token T;
	const ExprNode* Op1;
};
NodeCalcHelper operator<(const ExprNode* Op1, Token R)
{
	return NodeCalcHelper{ R, Op1 };
}
ExprNode* operator>(const NodeCalcHelper& L, ExprNode* Op2)
{
	return CreateNode(L.T, (ExprNode*)L.Op1, Op2);
}
struct FuncTokenHelper
{
	Token T;///< Stores the function/operator token

	/**
	 * @brief Returns the stored token
	 * @return const Token& Reference to the stored token
	 */
	const Token& operator()()const { return T; }

	/**
	 * @brief Creates a function node with single operand
	 * @param Op1 The operand node
	 * @return ExprNode* New function node
	 */
	ExprNode* operator()(const ExprNode* Op1)
	{
		return CreateNode(T, (ExprNode*)Op1, nullptr);
	}

	/**
	* @brief Creates a function node with two operands
	* @param Op1 First operand node
	* @param Op2 Second operand node
	* @return ExprNode* New function node
	*/
	ExprNode* operator()(const ExprNode* Op1, const ExprNode* Op2)
	{
		return CreateNode(T, (ExprNode*)Op1, (ExprNode*)Op2);
	}
};

// Global operator token definitions
// -purpose: Represents addition operator
Token ADD{ '+' };  
// -purpose: Represents subtraction operator
Token SUB{ '-' };  
// -purpose: Represents multiplication operator
Token MUL{ '*' };  
// -purpose: Represents division operator
Token DIV{ '/' };  
// -purpose: Represents power operator
Token POW{ '^' };  


// Macro helpers for operator token construction
// Helper for addition operator token
#define Add <ADD>  
// Helper for subtraction operator token
#define Sub <SUB>  
// Helper for multiplication operator token
#define Mul <MUL>  
 // Helper for division operator token
#define Div <DIV> 
// Helper for power operator token
#define Pwr <POW>  

// Function token helpers initialization
FuncTokenHelper Ln{ Token{FUNC_ln, AsFuncID{}} };
FuncTokenHelper Log{ Token{FUNC_log, AsFuncID{}} };
FuncTokenHelper Sin{ Token{FUNC_sin, AsFuncID{}} };
FuncTokenHelper Cos{ Token{FUNC_cos, AsFuncID{}} };
FuncTokenHelper Tan{ Token{FUNC_tan, AsFuncID{}} };
FuncTokenHelper Pow{ Token{FUNC_pow, AsFuncID{}} };
FuncTokenHelper Exp{ Token{FUNC_exp, AsFuncID{}} };
FuncTokenHelper Sinh{ Token{FUNC_sinh, AsFuncID{}} };
FuncTokenHelper Cosh{ Token{FUNC_cosh, AsFuncID{}} };

// Constant node creation macro
// -purpose: Shortcut for creating integer constant nodes
#define Const(x) CreateNode(Token(int(x)))

/**
 * @brief Represents a rational number with simplification capabilities
 */
struct Fraction
{
	int N, D;//Numerator, Denominator

	/**
	 * @brief Constructs a fraction with automatic simplification
	 * @param N Numerator
	 * @param D Denominator
	 */
	Fraction(int N, int D) : N(N), D(D) { Simplify(); }

	/**
	 * @brief Constructs whole number fraction
	 * @param N Integer value
	 */
	Fraction(int N) : N(N), D(1) {}

	/**
	 * @brief Simplifies fraction using GCD
	 */
	void Simplify()
	{
		if (!D)// Handle division by zero
		{
			if(!DividedbyZero)puts("Runtime Error: Divided by 0");
			DividedbyZero = true;
			return;
		}
		if (!N)D = 1;// Zero case
		else
		{
			int G = std::gcd(N, D);// C++17 cross-platform GCD
			N /= G;
			D /= G;
		}
	}

	// Arithmetic operators overloaded for fraction handling
	Fraction operator+(const Fraction& R) const
	{
		return Fraction(N * R.D + R.N * D, D * R.D);
	}
	Fraction operator-(const Fraction& R) const
	{
		return Fraction(N * R.D - R.N * D, D * R.D);
	}
	Fraction operator*(const Fraction& R) const
	{
		return Fraction(N * R.N, D * R.D);
	}
	Fraction operator/(const Fraction& R) const
	{
		return Fraction(N * R.D, D * R.N);
	}
	Fraction operator^(const Fraction& R) const
	{
		return Fraction((int)pow(N, ((double)R.N) / R.D), (int)pow(D, ((double)R.N) / R.D));
	}
	Fraction& operator+=(const Fraction& R)
	{
		*this = *this + R;
		return *this;
	}
	Fraction& operator-=(const Fraction& R)
	{
		*this = *this - R;
		return *this;
	}
	Fraction& operator*=(const Fraction& R)
	{
		*this = *this * R;
		return *this;
	}
	Fraction& operator/=(const Fraction& R)
	{
		*this = *this / R;
		return *this;
	}
	bool operator==(const Fraction& R) const { return N == R.N && D == R.D; }
	bool operator==(int R) const { return N == R && D == 1; }

	/**
	 * @brief Converts fraction to expression tree node
	 * @return ExprNode* Tree representation (n/d or integer if d=1)
	 */
	ExprNode* ToNode() const { return D == 1 ? Const(N) : (Const(N) Div Const(D)); }
};

/**
 * @brief Extracts greatest common divisor from a range of fractions
 * @tparam It Iterator type
 * @tparam Se Sentinel type
 * @param Begin Start iterator
 * @param End End iterator
 * @return Fraction The GCD of all fractions in range
 */
template<typename It, typename Se>
Fraction ExtractGCD(It Begin, Se End)
{
	int N0 = Begin->N, D0 = Begin->D;
	for (auto p = Begin; p != End; ++p)
	{
		N0 = std::gcd(N0, p->N); // Numerator GCD
		D0 = std::lcm(D0, p->D); // Denominator LCM
	}
	return Fraction(std::abs(N0), std::abs(D0));
}

/**
 * @brief Extracts GCD between two fractions
 * @param F1 First fraction
 * @param F2 Second fraction
 * @return Fraction The GCD fraction
 */
Fraction ExtractGCD(Fraction F1, Fraction F2)
{
	if (F1 == 0 || F2 == 0)return Fraction(0);
	return Fraction(std::gcd(F1.N, F2.N), std::lcm(F1.D, F2.D));
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//----------------------TREE TRAVERSE HELPERS----------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/*
- Purpose: Temporarily stores nodes during tree traversal operations.
- Usage: Various simplification and manipulation functions use this
		 vector to collect nodes of specific types or properties
		 for further processing.
*/
std::vector<ExprNode*> TraverseSeries;

/**
 * @brief Recursive implementation for collecting tree nodes matching required parent type
 * @param Series[out] Vector to store collected nodes
 * @param pNode Current node being processed
 * @param RequiredParentType Token type that parent nodes must match for collection
 */
void TraverseTreeNodes_Impl(std::vector<ExprNode*>& Series, const ExprNode* pNode, Token RequiredParentType)
{
	if (!pNode)return;

	// Match parent type condition
	if (pNode->V == RequiredParentType)
	{
		// Recursively process children for additive/multiplicative nodes
		if (pNode->HasOp0())TraverseTreeNodes_Impl(Series, pNode->L(), RequiredParentType);
		if (pNode->HasOp1())TraverseTreeNodes_Impl(Series, pNode->R(), RequiredParentType);
	}
	// Collect non-parent-type nodes
	else Series.push_back((ExprNode*)pNode);
}

/**
 * @brief Initializes traversal of tree nodes with specified parent type
 * @param Series[out] Vector to store collected nodes
 * @param pNode Root node of the subtree to traverse
 * @param RequiredParentType Token type that parent nodes must match
 */
void TraverseTreeNodes(std::vector<ExprNode*>& Series, const ExprNode* pNode, Token RequiredParentType)
{
	Series.clear();
	TraverseTreeNodes_Impl(Series, pNode, RequiredParentType);
}

/**
 * @brief Recursive implementation for counting tree nodes matching parent type
 * @param Result[out] Counter for matching nodes
 * @param pNode Current node being processed
 * @param RequiredParentType Token type that parent nodes must match
 */
void TraverseCountTreeNodes_Impl(int& Result, const ExprNode* pNode, Token RequiredParentType)
{
	if (!pNode)return;

	// Match parent type condition
	if (pNode->V == RequiredParentType)
	{
		// Recursively count children for additive/multiplicative nodes
		if (pNode->HasOp0())TraverseCountTreeNodes_Impl(Result, pNode->L(), RequiredParentType);
		if (pNode->HasOp1())TraverseCountTreeNodes_Impl(Result, pNode->R(), RequiredParentType);
	}
	// Increment counter for non-parent-type nodes
	else ++Result;
}

/**
 * @brief Counts nodes matching specified parent type in subtree
 * @param pNode Root node of the subtree to count
 * @param RequiredParentType Token type that parent nodes must match
 * @return int Total count of matching nodes
 */
int TraverseCountTreeNodes(const ExprNode* pNode, Token RequiredParentType)
{
	int Result = 0;
	TraverseCountTreeNodes_Impl(Result, pNode, RequiredParentType);
	return Result;
}

// Macro for type-based tree traversal
// -purpose: Simplifies traversal code for specific node types
// -usage: Expands to traversal loop using global TraverseSeries vector
#define TraverseType(Node, Ty, Idx) \
TraverseTreeNodes(TraverseSeries, Node, Ty);\
ExprNode* Idx{TraverseSeries[0]};\
for(size_t _I=0;_I<TraverseSeries.size();Idx=TraverseSeries[++_I])

// Macro for local type-based traversal
// -purpose: Enables type-specific traversal with local storage
// -usage: Creates local vector and loop for traversal context
#define TraverseTypeLocal(Cont, Node, Ty, Idx) \
std::vector<ExprNode*> Cont;\
TraverseTreeNodes(Cont, Node, Ty);\
ExprNode* Idx{Cont[0]};for(size_t _I=0;_I<Cont.size();Idx=Cont[++_I])

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-------------------------NODE OPERATIONS-------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
* @brief Computes the hash value of this node.
*
* @return ExprHash The hash value of the node.
*/
ExprHash ExprNode::Hash() const
{
	//Considering exchangeable + and * and commutative property
	if (V == ADD || V == MUL)
	{
		ExprHash H = V.Hash();
		TraverseTypeLocal(C, this, V, p)
			H += p ? TransformHash(p->Hash()) : 0;
		if (EnableDebugData) { printf("Size=%zu Hash()=%016llX Tree: ",TraverseSeries.size(), H), PrintTree(); putchar('\n'); }
		return H;
	}
	else
	{
		auto H= V.Hash()
			+ (L() ? TransformHash(L()->Hash()) : 0)
			+ (R() ? TransformHash(R()->Hash()) : 0);
		if (EnableDebugSimplifyII) { printf("Hash()=%016llX Tree: ", H); PrintTree(); putchar('\n'); }
		return H;
	}
}

/**
* @brief Checks if this node represents a squared term.
*
* @return bool True if the node is a squared term, false otherwise.
*/
bool ExprNode::IsSquare() const
{
	return V.IsPOW() && V1() == Token((int)2);
}

/**
* @brief Checks if this node represents a constant value.
*
* @return bool True if the node is a constant, false otherwise.
*/
bool ExprNode::IsConst() const
{
	if (V.Ty == Token::Int)return true;
	else if (V.Ty == Token::Variable)return false;
	else if (V.Ty == Token::Function)return false;
	else return (L() ? L()->IsConst() : true) && (R() ? R()->IsConst() : true);
}

/**
* @brief Checks if this node represents a constant value excluding radicals.
*
* @return bool True if the node is a constant in the specified form, false otherwise.
*/
bool ExprNode::IsConstII() const
{
	if (V.Ty == Token::Int)return true;
	else if (V.Ty == Token::Variable)return false;
	else if (V.Ty == Token::Function)return false;
	else if (V.ID == '^')return false;
	else return (L() ? L()->IsConstII() : true) && (R() ? R()->IsConstII() : true);
}

/**
* @brief Returns the operator precedence level of this node.
*
* @param ConsiderUsedNode Whether to consider if the node is already used.
* @return int The precedence level of the operator.
*/
int ExprNode::OprLevel(bool ConsiderUsedNode) const
{
	if (V.Ty == Token::Operator)
	{
		if (L() && !ConsiderUsedNode)return 0;
		else if (V.ID == '+' || V.ID == '-')return 1;
		else if (V.ID == '*' || V.ID == '/')return 2;
		else if (V.ID == '^')return 3;
		else return 114514;//never occur
	}
	else return 0;
}

/**
* @brief Prints the node's information for debugging purposes.
*/
void ExprNode::DebugPrint() const
{
	/*Token V;
	ExprNode* Prev, *Next;
	ExprNode* Operand[2];*/
	if (V.Ty == Token::Function || V.Ty == Token::Operator)
	{
		printf("<%04X>%s:{", unsigned(((size_t)this) % 0xFFFF), V.GetText());
		if (L())L()->DebugPrint();
		printf("}{");
		if (R())R()->DebugPrint();
		printf("}");
	}
	else printf("<%04X>%s", unsigned(((size_t)this) % 0xFFFF), V.GetText());
}

/**
* @brief Prints the subtree rooted at this node in a tree-like structure.
*
* @param Parent The parent node of this node.
* @param PrintedCount A counter for the number of nodes printed.
* @param IsLeft Indicates if this node is the left child of its parent.
*/
void ExprNode::PrintTree(const ExprNode* Parent, int& PrintedCount, bool IsLeft) const
{
	int tpc = PrintedCount;
	switch (V.Ty)
	{
	case Token::Int:
		if (V.ID >= 0 || !Parent)printf("%d", V.ID);
		else printf("(%d)", V.ID);
		break;
	case Token::Variable:
		printf("%s", V.GetText()); break;
	case Token::Operator:
	{
		bool NeedsBracket;
		bool Neg = V.ID == '*' && V0() == Token((int)-1);
		if (Parent && Parent->V.Ty == Token::Operator)
		{
			int PL = Parent->OprLevel(true);
			int ML = OprLevel(true);
			NeedsBracket = (PL > ML || (PL == ML && PL &&
				((Parent->V.ID == '-' && !IsLeft) || (Parent->V.ID == '/' && !IsLeft) || (Parent->V.ID == '^' && IsLeft))));
		}
		else NeedsBracket = false;
		NeedsBracket = !(Neg && !tpc) && (NeedsBracket || (Parent && Neg));
		if (NeedsBracket)putchar('(');
		//-1*x -> -x
		if (Neg)putchar('-');
		else
		{
			L()->PrintTree(this, PrintedCount, true);
			if (!(V.ID == '*' && Ty0() == Token::Int && Ty1() != Token::Int))putchar(V.ID);
		}
		R()->PrintTree(this, PrintedCount, false);
		if (NeedsBracket)putchar(')');
		break;
	}
	case Token::Function:
		printf("%s(", V.GetText());
		L()->PrintTree(this, PrintedCount, true);
		if (Funcs[V.ID].NParam == 2)
		{
			putchar(',');
			R()->PrintTree(this, PrintedCount, false);
		}
		putchar(')');
		break;
	}
	++PrintedCount;
}
void ExprNode::DebugPrintTraverse(const ExprNode* End) const
{
	for (auto V = this; V != End; V = V->Next)
	{
		V->DebugPrint();
		putchar('\n');
	}
}

/**
* @brief Creates a copy of this node.(Only Token & Operand)
*
* @return ExprNode* A duplicate of this node.
*/
ExprNode* ExprNode::Duplicate() const
{
	auto N = CreateNode();
	N->V = V;
	if (L())N->L() = L()->Duplicate();
	if (R())N->R() = R()->Duplicate();
	return N;
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//-----------------------PARTIAL DERIVATIVE------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
* @brief Computes the partial derivative of this node with respect to a variable.
*
* @param DX The ID of the variable with respect to which to differentiate.
* @return ExprNode* The resulting node after differentiation.
*/
ExprNode* ExprNode::Partial(int DX) const
{
	switch (V.Ty)
	{
		// Derivative of constant is 0
	case Token::Int: return Const(0);
		// dx/dx=1, dy/dx=0
	case Token::Variable: return V.ID == DX ? Const(1) : Const(0);
		// Handle operator nodes
	case Token::Operator: return PartialOpr(DX);
		// Use function-specific rules
	case Token::Function: return Funcs[V.ID].Derivative(L(), R(), DX);
		// Fallback for undefined types
	default:return Const(0);
	}
}

/**
* @brief Computes the partial derivative for operator nodes.
*
* @param DX The ID of the variable with respect to which to differentiate.
* @return ExprNode* The resulting node after differentiation.
*/
ExprNode* ExprNode::PartialOpr(int DX) const
{
	switch (V.ID)
	{
		// d(f+g)/dx = f' + g'
	case '+':return DX_add(L(), R(), DX);
		// d(f-g)/dx = f' - g'
	case '-':return DX_sub(L(), R(), DX);
		// d(f*g)/dx = f'*g + f*g'
	case '*':return DX_mul(L(), R(), DX);
		// d(f/g)/dx = (f'g - fg')/g
	case '/':return DX_div(L(), R(), DX);
	case '^':return DX_pow(L(), R(), DX);
	default: 
		// Safety fallback
		("Fatal Error: Unknown Operator %c", V.ID); 
		return Duplicate();//How could this happen?
	}
}

// Derivative rule macros (avoid code duplication)
// -purpose: Clone left operand
#define F  (Op1->Duplicate())
// -purpose: Derivative of left operand
#define DF (Op1->Partial(DX))
// -purpose: Clone right operand
#define G  (Op2->Duplicate())
// -purpose: Derivative of right operand
#define DG (Op2->Partial(DX))

/**
 * @brief Derivative rule for addition: d(f+g)/dx = df/dx + dg/dx
 * 
 * @param Op1 Left operand expression (f)
 * @param Op2 Right operand expression (g)
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_add(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//f+g -> f'+g'
	return DF Add DG;
}

/**
 * @brief Derivative rule for subtraction: d(f-g)/dx = df/dx - dg/dx
 * @param Op1 Left operand expression (f)
 * @param Op2 Right operand expression (g)
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_sub(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//f-g -> f'-g'
	return DF Sub DG;
}

/**
 * @brief Derivative rule for multiplication: d(f*g)/dx = f'*g + f*g'
 * @param Op1 Left operand expression (f)
 * @param Op2 Right operand expression (g)
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_mul(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//f*g -> f'*g+f*g'
	return (DF Mul G)Add(F Mul DG);
}

/**
 * @brief Derivative rule for division: d(f/g)/dx = (f'g - fg')/g^2
 * @param Op1 Numerator expression (f)
 * @param Op2 Denominator expression (g)
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_div(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//f/g -> (f'*g-f*g')/(g^2)
	return ((DF Mul G)Sub(F Mul DG))Div(G Pwr Const(2));
}

/**
 * @brief Derivative rule for natural logarithm: d(ln(f))/dx = f'/f
 * @param Op1 Inner function expression (f)
 * @param Op2 (Unused) Placeholder for interface consistency
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_ln(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//ln(f) -> f'/f
	return DF Div F;
}

/**
 * @brief Derivative rule for logarithm: d(log_b(f))/dx = (ln(f)/ln(b))'
 * @param Op1 Base expression (b)
 * @param Op2 Argument expression (f)
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree using quotient rule
 */
ExprNode* DX_log(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//log(f,g)=ln(g)/ln(f)
	return DX_div(Ln(Op2), Ln(Op1), DX);
}

/**
 * @brief Derivative rule for cosine: d(cos(f))/dx = -f'*sin(f)
 * @param Op1 Inner function expression (f)
 * @param Op2 (Unused) Placeholder for interface consistency
 * @param DX Target variable ID
 * @return ExprNode* Resulting derivative tree
 */
ExprNode* DX_cos(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//cos(f) -> 0-f'*sin(f)
	return Const(0) Sub(DF Mul Sin(F));
}

/**
 * @brief Computes the derivative of the sine function
 * @param Op1 The inner function expression (f in sin(f))
 * @param Op2 Unused parameter (maintained for interface consistency)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree: cos(f) * f'
 */
ExprNode* DX_sin(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//sin(f) -> f'*cos(f)
	return DF Mul Cos(F);
}

/**
 * @brief Computes the derivative of the tangent function
 * @param Op1 The inner function expression (f in tan(f))
 * @param Op2 Unused parameter (interface consistency)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree: f' / cos(f)^2
 */
ExprNode* DX_tan(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//tan(f) -> f'/cos^2(f)
	return DF Div(Cos(F) Pwr Const(2));
}

/**
 * @brief Computes the derivative of the power function (f^g)
 * @param Op1 Base expression (f in f^g)
 * @param Op2 Exponent expression (g in f^g)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree using logarithmic differentiation
 */
ExprNode* DX_pow(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//f^g=exp(g*lnf)
	return DX_exp(Op2 Mul Ln(Op1), nullptr, DX);
}

/**
 * @brief Computes the derivative of the exponential function (e^f)
 * @param Op1 The exponent expression (f in exp(f))
 * @param Op2 Unused parameter (interface consistency)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree: exp(f) * f'
 */
ExprNode* DX_exp(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//exp(f) -> f'*exp(f)
	return DF Mul Exp(F);
}

/**
 * @brief Computes the derivative of the hyperbolic sine function
 * @param Op1 The inner function expression (f in sinh(f))
 * @param Op2 Unused parameter (interface consistency)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree: cosh(f) * f'
 */
ExprNode* DX_sinh(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//sinhf -> f'*coshf
	return DF Mul Cosh(F);
}

/**
 * @brief Computes the derivative of the hyperbolic cosine function
 * @param Op1 The inner function expression (f in cosh(f))
 * @param Op2 Unused parameter (interface consistency)
 * @param DX The variable ID to differentiate against
 * @return ExprNode* Derivative expression tree: sinh(f) * f'
 */
ExprNode* DX_cosh(const ExprNode* Op1, const ExprNode* Op2, int DX)
{
	//coshf -> f'*sinhf
	return DF Mul Sinh(F);
}

// Macro cleanup to prevent namespace pollution
#undef F  
#undef DF 
#undef G 
#undef DG 

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//------------------------SIMPLIFY HELPERS-------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief Checks if two expression nodes are structurally equivalent
 * @param L First node to compare
 * @param R Second node to compare
 * @return bool True if nodes have identical structure and values
 */
bool Equal(const ExprNode* L, const ExprNode* R)
{
	auto H1 = L ? L->Hash() : 0, H2 = R ? R->Hash() : 0;
	// Compare hash values for quick equivalence check
	return H1 == H2;
}

/**
 * @brief Extracts constant value from constant expression subtree
 * @param pNode Root node of the subtree to evaluate
 * @return Fraction The numerical value as a reduced fraction
 */
Fraction ExtractConst(const ExprNode* pNode)
{
	if (pNode->V.Ty == Token::Operator)
	{
		switch (pNode->V.ID)
		{
		case '+':return ExtractConst(pNode->L()) + ExtractConst(pNode->R());
		case '-':return ExtractConst(pNode->L()) - ExtractConst(pNode->R());
		case '*':return ExtractConst(pNode->L()) * ExtractConst(pNode->R());
		case '/':return ExtractConst(pNode->L()) / ExtractConst(pNode->R());
		default:return Fraction(0);
		}
	}
	// Return integer value for constant nodes
	else return Fraction(pNode->V.ID);
}

/**
 * @brief Extracts coefficients from multiplicative expressions
 * @param pNode Root node of the expression to analyze
 * @return pair<bool, Fraction> (modified status, extracted coefficient)
 */
std::pair<bool, Fraction> ExtractCoefficient(const ExprNode* pNode)
{
	bool Simplify_FoldConst(ExprNode * &pNode);
	Fraction F(1);
	bool Changed = false;

	// Traverse multiplicative terms
	TraverseType(pNode, MUL, p)
	{
		if (p->V.Ty == Token::Int && p->V.ID != 1)
		{
			F = F * Fraction(p->V.ID);
			p->V = Token(int(1));
			Changed = true;
		}
		else if (p->IsConstII())
		{
			// Handle complex constants
			auto alt = CreateNode();
			alt->Copy(p);
			Simplify_FoldConst(alt);
			F = F * ExtractConst(alt);
			ReleaseTree(alt);
			ClearNode(p);
			p->V = Token(int(1));
			Changed = true;
		}
	}
	return { Changed, F };
}

/**
 * @brief Merges identical factors in power expressions
 * @param pNode Root node of the expression to process
 * @param Tg Map tracking factor hashes to detect duplicates
 * @return bool True if any merges occurred
 */
bool MergeSame(ExprNode*& pNode, std::map<ExprHash, ExprNode**>& Tg)
{
	if constexpr (EnableDebugSimplifyII) { printf("GetMS: "); pNode->PrintTree(); putchar('\n'); }
	//(y*z)^x=y^x*z^x
	//Put it in Rotate causes infinite loop
	if (pNode->V == POW && pNode->V0() == MUL)
	{
		//(y*z)^x -> (y*z)*x
		pNode->V.ID = '*';
		auto Z = pNode->L()->R();
		pNode->L()->R() = pNode->R()->Duplicate();//(y*z)*x -> (y*x)*x z
		pNode->L()->V.ID = '^';
		pNode->R() = Z Pwr pNode->R();
	}
	if (pNode->V == MUL)
	{
		bool V = MergeSame(pNode->L(), Tg);
		V |= MergeSame(pNode->R(), Tg);
		return V;
	}
	else if (pNode->IsConst())return false;
	else if (pNode->V == POW)
	{
		if constexpr (EnableDebugSimplifyII) { printf("GetP: "); pNode->PrintTree(); putchar('\n'); }
		auto H = pNode->L()->Hash();
		if constexpr (EnableDebugSimplifyII) { printf("Hash %016llX Tree: ", H); pNode->L()->PrintTree(); putchar('\n'); }
		auto it = Tg.find(H);
		if (it == Tg.end())
		{
			// Register new factor
			Tg[H] = &pNode;
			return false;
		}
		else
		{
			// Merge duplicate factors
			auto& T = (*(it->second));
			auto B = pNode->R();
			pNode->R() = Const(0);
			if (T->V == POW)T->R() = T->R() Add B;
			else T = T Pwr(Const(1) Add B);
			return true;
		}
	}
	else
	{
		auto H = pNode->Hash();
		if constexpr (EnableDebugSimplifyII) { printf("Hash %016llX GetM: ", H); pNode->PrintTree(); putchar('\n'); }
		auto it = Tg.find(H);
		if (it == Tg.end())
		{
			// Register new factor
			Tg[H] = &pNode;
			return false;
		}
		else
		{
			// Merge duplicate factors
			auto& T = (*(it->second));
			ReleaseTree(pNode);
			pNode = Const(1);
			if (T->V == POW)T->R() = T->R() Add Const(1);
			else T = T Pwr(Const(2));
			return true;
		}
	}
}

/**
 * @brief Populates factor map with multiplicative components
 * @param pNode Root node of the expression to analyze
 * @param F Factor map to populate
 */
void FillFactorMap(ExprNode*& pNode, std::map<ExprHash, ExprNode**>& F)
{
	if (pNode->V == MUL)
	{
		FillFactorMap(pNode->L(), F);
		FillFactorMap(pNode->R(), F);
	}
	else
	{
		// Record non-multiplicative components
		F[pNode->Hash()] = &pNode;
	}
}

/**
 * @brief Rotates constant coefficients to front of multiplicative expressions
 * @param pNode Root node of the expression to modify
 * @return bool True if rotation occurred
 */
bool RotateCoefficient(ExprNode*& pNode)
{
	if (!pNode)return false;
	if (Extracted.find(pNode->Hash()) != Extracted.end())return false;
	if (pNode->V.Ty == Token::Int)return false;
	auto F = ExtractCoefficient(pNode).second;
	auto pC = F.ToNode();

	// Prepend coefficient to expression
	pNode = pC Mul pNode;
	Extracted.insert(pNode->Hash());
	return true;
}

/**
 * @brief Replaces an expression node with an integer constant node
 * @param x The integer value to replace with
 * @param pNode[in,out] Reference to the node pointer to be replaced
 *
 * - Releases existing node memory through ReleaseTree
 * - Creates new constant node with specified value
 * - Modifies the original pointer to point to new node
 */
void ReplaceInt(int x, ExprNode*& pNode)
{
	ReleaseTree(pNode);
	pNode = CreateNode(Token(x));
}

// -purpose: Tracks relationships between common factors during simplification  
// -usage: Stores mapping of hash values to node pointers for factor merging
struct CommonFactor
{
	ExprNode** ppi;
	ExprNode** ppj;
	ExprNode* pi;
	ExprNode* pj;
};

void GetCommonFactor(std::map<ExprHash, ExprNode**>& I,
	std::map<ExprHash, ExprNode**>& J, std::map<ExprHash, CommonFactor>& T)
{
	T.clear();
	for (auto& [k, v] : I)
	{
		auto it = J.find(k);
		if (it != J.end())
		{
			T[k] = { v, it->second, *v, *it->second };
		}
	}
}

/**
 * @brief Creates a multiplied factor tree from duplicate components
 * @param Raw Vector of factor nodes to combine
 * @param Begin Starting index in vector
 * @param End Ending index in vector (exclusive)
 * @return ExprNode* Balanced binary tree of multiplication operations
 *
 * - Uses divide-and-conquer to build balanced multiplication tree
 * - Example: [a,b,c,d] -> ((a*b)*(c*d))
 */
ExprNode* DuplicateFactorImpl(std::vector<ExprNode*>& Raw, int Begin, int End)
{
	if (End - Begin == 1)return Raw[Begin];
	else
	{
		int Mid = (Begin + End) / 2;
		return DuplicateFactorImpl(Raw, Begin, Mid) Mul DuplicateFactorImpl(Raw, Mid, End);
	}
}

/**
 * @brief Creates optimized multiplication tree from common factors
 * @param T Map of common factor relationships
 * @return ExprNode* Combined multiplication expression tree
 */
ExprNode* DuplicateFactor(std::map<ExprHash, CommonFactor>& T)
{
	std::vector<ExprNode*> BuildRaw;
	BuildRaw.reserve(T.size());
	for (auto& [k, v] : T)BuildRaw.push_back(v.pi->Duplicate());
	return DuplicateFactorImpl(BuildRaw, 0, (int)BuildRaw.size());
}


//-----------------------------------------------------------------
//-----------------------------------------------------------------
//------------------------SIMPLIFY FUNCTIONS-----------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------


/**
 * @brief Merges power terms with identical bases (x^a * x^b -> x^(a+b)
 * @param pNode[in,out] Root node of the expression subtree to process
 * @return bool True if any power merging occurred
 */
bool Simplify_MergePower(ExprNode*& pNode)
{
	std::map<ExprHash, ExprNode**> Tg;
	if constexpr (EnableDebugSimplifyII) { printf("\nMP: "); pNode->PrintTree(); putchar('\n'); }
	
	// Use hash map to detect duplicate bases
	return MergeSame(pNode, Tg);
}

/**
 * @brief Handles special cases with 0/1 exponents (x^0 -> 1, 0^x -> 0, etc)
 * @param pNode[in,out] Power expression node to simplify
 * @return bool True if simplification occurred
 */
bool Simplify_01Pwr(ExprNode*& pNode)
{
	if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 0)//x^0=1
	{
		ReplaceInt(1, pNode);
		return true;
	}
	else if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)//0^x=0
	{
		ReplaceInt(0, pNode);
		return true;
	}
	else if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 1)//1^x=1
	{
		ReplaceInt(1, pNode);
		return true;
	}
	else if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 1)//x^1=x
	{
		ReleaseNode(pNode);
		pNode = pNode->L();
		return true;
	}
	return false;
}

/**
 * @brief Simplifies expressions containing 0/1 constants
 * @param pNode[in,out] Root node of the subtree to process
 * @return bool True if any 0/1 simplification occurred
 */
bool Simplify_01(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;
	Changed |= Simplify_01(pNode->L());
	Changed |= Simplify_01(pNode->R());
	switch (pNode->V.Ty)
	{
	case Token::Int:break;
	case Token::Variable:break;
	case Token::Function:
	{
		switch (pNode->V.ID)
		{
		case FUNC_ln:
			if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 1)
			{
				ReplaceInt(0, pNode);
				Changed = true;
			}
			break;
		case FUNC_log:break;//It won't happen in derived functions
		case FUNC_exp:
		case FUNC_cos:
		case FUNC_cosh:
			if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)//cos(0)=1 exp(0)=1
			{
				ReplaceInt(1, pNode);
				Changed = true;
			}
			break;
		case FUNC_sin:
		case FUNC_tan:
		case FUNC_sinh:
			if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)
			{
				ReplaceInt(0, pNode);
				Changed = true;
			}
			break;
		case FUNC_pow:
			Changed |= Simplify_01Pwr(pNode); break;
		default:break;
		}
		break;
	}
	case Token::Operator:
	{
		switch (pNode->V.ID)
		{
			// Handle additive identities
		case '+':
			if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)
			{
				ReleaseNode(pNode);
				pNode = pNode->R();
				Changed = true;
			}
			else if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 0)
			{
				ReleaseNode(pNode);
				pNode = pNode->L();
				Changed = true;
			}break;
		case '-':
			if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 0)
			{
				ReleaseNode(pNode);
				pNode = pNode->L();
				Changed = true;
			}
			else if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)
			{
				ReplaceInt(-1, pNode->L());
				pNode->V.ID = '*';
				Changed = true;
			}
			break;
			// Handle multiplicative identities
		case '*':
			if ((pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)
				|| (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 0))
			{
				ReplaceInt(0, pNode);
				Changed = true;
			}
			else if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 1)
			{
				ReleaseNode(pNode);
				pNode = pNode->R();
				Changed = true;
			}
			else if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 1)
			{
				ReleaseNode(pNode);
				pNode = pNode->L();
				Changed = true;
			}
			break;
		case '/':
			if (pNode->HasOp0() && pNode->Ty0() == Token::Int && pNode->ID0() == 0)
			{
				ReplaceInt(0, pNode);
				Changed = true;
			}
			else if (pNode->HasOp1() && pNode->Ty1() == Token::Int && pNode->ID1() == 1)
			{
				ReleaseNode(pNode);
				pNode = pNode->L();
				Changed = true;
			}break;
		case '^':
			Changed |= Simplify_01Pwr(pNode); break;
		default:break;
		}
		break;
	}
	default:break;
	};
	return Changed;
}

/**
 * @brief Restructures expression tree for canonical form
 * @param pNode[in,out] Root node of the subtree to rotate
 * @return bool True if any structural changes occurred
 *
 * - Converts subtraction to addition with negative coefficients
 * - Flattens nested division/multiplication structures
 */
bool Simplify_Rotate(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;
	Changed |= Simplify_Rotate(pNode->L());
	Changed |= Simplify_Rotate(pNode->R());
	if (pNode->V == ADD)
	{
		//(a-b)+(c-d)=(a+c)-(b+d)
		if (pNode->L()->V == SUB && pNode->R()->V == SUB)
		{
			std::swap(pNode->L()->R(), pNode->R()->L());//(a-b)+(c-d)->(a-c)+(b-d)
			pNode->V.ID = '-';
			pNode->ID0() = '+';
			pNode->ID1() = '+';
		}
		//x+(y-z)=(x+y)-z
		else if (pNode->R()->V == SUB)
		{
			auto p = pNode->R();
			pNode->R() = p->L();
			p->L() = pNode;
			pNode = p;
			Changed = true;
		}
		//(x-y)+z=(x+z)-y
		else if (pNode->L()->V == SUB)
		{
			auto p = pNode->L();
			pNode->L() = p->L();
			p->L() = pNode;
			pNode = p;
			Changed = true;
		}

	}
	else if (pNode->V == SUB)
	{
		//(a-b)-(c-d)=(a+d)-(c+b)
		if (pNode->L()->V == SUB && pNode->R()->V == SUB)
		{
			std::swap(pNode->L()->R(), pNode->R()->R());//(a-b)-(c-d)->(a-d)-(c-b)
			pNode->ID0() = '+';
			pNode->ID1() = '+';
		}
		//x-(y-z)=(x+z)-y
		else if (pNode->R()->V == SUB)
		{
			pNode->V.ID = '+';//x-(y-z) -> x+(y-z)
			auto p = pNode->R();//x+(y-z) -> x+(y-z) y-z
			pNode->R() = p->R();//x+(y-z) y-z -> x+z y-z
			p->R() = p->L();//x+z y-z -> x+z y-y
			p->L() = pNode;//x+z y-y -> x+z (x+z)-y
			pNode = p;
			Changed = true;
		}
		//(x-y)-z=x-(y+z)
		else if (pNode->L()->V == SUB)
		{
			pNode->V.ID = '+';//(x-y)-z -> (x-y)+z
			auto p = pNode->L();//(x-y)+z -> (x-y)+z x-y
			pNode->L() = p->R();//(x-y)+z x-y -> y+z x-y
			p->R() = pNode;//y+z x-y -> y+z x-(y+z)
			pNode = p;
			Changed = true;
		}
	}
	else if (pNode->V == MUL)
	{
		//(a/b)*(c/d)=(a*c)/(b*d)
		if (pNode->L()->V == DIV && pNode->R()->V == DIV)
		{
			std::swap(pNode->L()->R(), pNode->R()->L());//(a-b)+(c-d)->(a-c)+(b-d)
			pNode->V.ID = '/';
			pNode->ID0() = '*';
			pNode->ID1() = '*';
		}
		//x*(y/z)=(x*y)/z
		else if (pNode->R()->V == DIV)
		{
			auto p = pNode->R();
			pNode->R() = p->L();
			p->L() = pNode;
			pNode = p;
			Changed = true;
		}
		//(x/y)*z=(x*z)/y
		else if (pNode->L()->V == DIV)
		{
			auto p = pNode->L(); //(x/y)*z ->  (x/y)*z x/y
			pNode->L() = p->L();//(x/y)*z ->  x*z x/y
			p->L() = pNode;//x*z (x/y) ->  x*z (x*z)/y
			pNode = p;
			Changed = true;
		}
	}
	else if (pNode->V == DIV)
	{
		//(a/b)/(c/d)=(a*d)/(c*b)
		if (pNode->L()->V == DIV && pNode->R()->V == DIV)
		{
			std::swap(pNode->L()->R(), pNode->R()->R());//(a-b)-(c-d)->(a-d)-(c-b)
			pNode->ID0() = '*';
			pNode->ID1() = '*';
		}
		//x/(y/z)=(x*z)/y
		else if (pNode->R()->V == DIV)
		{
			pNode->V.ID = '*';
			auto p = pNode->R();
			pNode->R() = p->R();
			p->R() = p->L();
			p->L() = pNode;
			pNode = p;
			Changed = true;
		}
		//(x/y)/z=x/(y*z)
		else if (pNode->L()->V == DIV)
		{
			pNode->V.ID = '*';
			auto p = pNode->L();
			pNode->L() = p->R();
			p->R() = pNode;
			pNode = p;
			Changed = true;
		}
	}
	else if (pNode->V.IsPOW())
	{
		//(x^a)^b=x^(a*b)
		if (pNode->V0().IsPOW())
		{
			auto p = pNode->L();//(x^a)^b -> (x^a)^b x^a
			pNode->L() = p->R();//(x^a)^b x^a -> a^b x^a
			pNode->V.ID = '*';//a^b x^a  -> a*b x^a
			p->R() = pNode;//a*b x^a -> a*b x^(a*b)
			pNode = p;
			Changed = true;
		}
	}
	else if (pNode->V == Pow())
	{
		pNode->V = POW;
		Changed = true;
	}
	else if (pNode->V == Log())
	{
		pNode = Ln(pNode->R()) Div Ln(pNode->L());
		Changed = true;
	}
	return Changed;
}

/**
 * @brief Applies all 0/1 simplifications until no more changes
 * @param pNode[in,out] Root node of the expression tree
 */
bool Simplify_All01(ExprNode*& pNode)
{
	bool Changed = false;
	Changed |= Simplify_Rotate(pNode);
	Changed |= Simplify_01(pNode);
	return Changed;
}

/**
 * @brief Simplifies trigonometric and hyperbolic identities
 * @param pNode[in,out] Root node of the expression subtree
 * @return bool True if any special function simplification occurred
 *
 * - Simplifies trigonometric conversions
 * - Converts log/exp combinations to algebraic forms
 */
bool Simplify_SpecialFuncs(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;
	Changed |= Simplify_SpecialFuncs(pNode->L());
	Changed |= Simplify_SpecialFuncs(pNode->R());
	switch (pNode->V.Ty)
	{
	case Token::Function:
	{
		switch (pNode->V.ID)
		{
		case FUNC_exp:
		{
			//exp(ln(f))=f
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Function && p->V.ID == FUNC_ln)
				{
					auto Base = p->L();
					p->L() = nullptr;
					p->V = Token((int)1);
					pNode->V = POW;
					pNode->R() = pNode->L();
					pNode->L() = Base;
					Changed = true;
					break;
				}
			}break;
		}
		case FUNC_ln:
		{
			//ln(exp(f))=f
			//ln(a^b)=b*ln(a)
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V == Exp())
				{
					auto Base = p->L();
					p->L() = nullptr;
					p->V = Token((int)1);
					pNode->V = ADD;
					pNode->L() = Ln(pNode->L());
					pNode->R() = Base;
					Changed = true;
					break;
				}
			}
			p = pNode->L();
			if (p->V.IsPOW())
			{
				auto U = p->R();
				p->R() = Const(1);
				pNode = U Mul pNode;
				Changed = true;
			}break;
		}
		default:break;
		}break;
	}
	case Token::Operator:
	{
		switch (pNode->V.ID)
		{
		case '/':
			//sin(x)/cos(x)=tan(x)
			if (pNode->V0() == Sin() && pNode->V1() == Cos()
				&& Equal(pNode->L()->L(), pNode->R()->L()))
			{
				pNode->V = Tan();
				auto X = pNode->L()->L();
				ReleaseNode(pNode->L());
				ReleaseTree(pNode->R());
				pNode->L() = X;
				Changed = true;
				break;
			}
			//cos(x)/sin(x)=1/tan(x)
			else if (pNode->V0() == Cos() && pNode->V1() == Sin()
				&& Equal(pNode->L()->L(), pNode->R()->L()))
			{
				pNode->V1() = Tan();
				ReleaseTree(pNode->L());
				pNode->L() = Const(1);
				Changed = true;
				break;
			}
			break;
		case '+':
			//sin(x)^2+cos(x)^2=1
			//cos(x)^2+sin(x)^2=1
			if (pNode->L()->IsSquare() && pNode->R()->IsSquare()
				&& ((pNode->L()->V0() == Sin() && pNode->R()->V0() == Cos()) || (pNode->L()->V0() == Cos() && pNode->R()->V0() == Sin()))
				&& Equal(pNode->L()->L()->L(), pNode->R()->L()->L()))
			{
				ReleaseTree(pNode);
				pNode = Const(1);
				Changed = true;
				break;
			}
			//sinh^2+1=cosh^2
			else if (pNode->V1() == Token(int(1))
				&& pNode->L()->IsSquare()
				&& pNode->L()->V0() == Sinh())
			{
				pNode->L()->L()->V = Cosh();
				ReleaseNode(pNode);
				ReleaseNode(pNode->R());
				pNode = pNode->L();
				Changed = true;
				break;
			}
			//1+sinh^2=cosh^2
			else if (pNode->V0() == Token(int(1))
				&& pNode->R()->IsSquare()
				&& pNode->R()->V0() == Sinh())
			{
				pNode->R()->L()->V = Cosh();
				ReleaseNode(pNode);
				ReleaseNode(pNode->L());
				pNode = pNode->R();
				Changed = true;
				break;
			}
			break;
		case '-':
			//cosh^2-sinh^2=1
			if (pNode->L()->IsSquare() && pNode->R()->IsSquare()
				&& pNode->L()->V0() == Cosh() && pNode->R()->V0() == Sinh()
				&& Equal(pNode->L()->L()->L(), pNode->R()->L()->L()))
			{
				ReleaseTree(pNode);
				pNode = Const(1);
				Changed = true;
				break;
			}
			//1-sin^2=cos^2
			else if (pNode->V0() == Token(int(1))
				&& pNode->R()->IsSquare()
				&& pNode->R()->V0() == Sin())
			{
				pNode->R()->L()->V = Cos();
				ReleaseNode(pNode);
				ReleaseNode(pNode->L());
				pNode = pNode->R();
				Changed = true;
				break;
			}
			//1-cos^2=sin^2
			else if (pNode->V0() == Token(int(1))
				&& pNode->R()->IsSquare()
				&& pNode->R()->V0() == Cos())
			{
				pNode->R()->L()->V = Sin();
				ReleaseNode(pNode);
				ReleaseNode(pNode->L());
				pNode = pNode->R();
				Changed = true;
				break;
			}
			//cosh^2-1=sinh^2
			else if (pNode->V1() == Token(int(1))
				&& pNode->L()->IsSquare()
				&& pNode->L()->V0() == Cosh())
			{
				pNode->L()->L()->V = Sinh();
				ReleaseNode(pNode);
				ReleaseNode(pNode->R());
				pNode = pNode->L();
				Changed = true;
				break;
			}
			break;
		default:break;
		}break;
	}
	default:break;
	}
	return Changed;
}

/**
 * @brief Normalizes negative sign distribution
 * @param pNode[in,out] Root node of the expression subtree
 * @return bool True if any negation pattern was modified
 *
 * - Converts subtraction to addition with negative coefficients
 * - Simplifies double negatives (-(-x) -> x)
 * - Moves negative signs to canonical positions in products
 */
bool Simplify_Neg(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;
	Changed |= Simplify_Neg(pNode->L());
	Changed |= Simplify_Neg(pNode->R());
	switch (pNode->V.Ty)
	{
	case Token::Operator:
	{
		switch (pNode->V.ID)
		{
		case '-':
		{
			//a-b=a+(-1)*b
			//a-(-1)*b=a+b
			bool NegR = false;
			TraverseType(pNode->R(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					NegR = !NegR;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			pNode->V.ID = '+';
			if (!NegR)pNode->R() = Const(-1) Mul pNode->R();
			break;
		}
		case '*':
		{
			//C*(a+b)=C*a+C*b
			if (pNode->Ty0() == Token::Int && pNode->V1() == ADD)
			{
				auto p = pNode->R();
				auto C = pNode->L();
				p->L() = C Mul p->L();
				p->R() = C->Duplicate() Mul p->R();
				ReleaseNode(pNode);
				pNode = p;
			}
			//(a+b)*C=C*a+C*b
			else if (pNode->Ty1() == Token::Int && pNode->V0() == ADD)
			{
				auto p = pNode->L();
				auto C = pNode->R();
				p->L() = C Mul p->L();
				p->R() = C->Duplicate() Mul p->R();
				ReleaseNode(pNode);
				pNode = p;
			}
			break;
		}
		case '/':
		{
			pNode->V.ID = '*';
			TraverseType(pNode->R(), MUL, p)
			{
				//C/x^y=C*x^((-1)*y)
				if (p->V == POW)
					p->R() = Const(-1) Mul p->R();
				//C/exp(x)=C*exp(-1*x)
				else if (p->V == Exp())
					p->L() = Const(-1) Mul p->L();
				//C/x=C*x^(-1)
				else
				{
					auto M = CreateNode(); M->Copy(p);
					auto Q = M Pwr Const(-1);
					p->Copy(Q);
				}
			}
			break;
		}
		default:break;
		}
		break;
	}
	case Token::Function:
	{
		switch (pNode->V.ID)
		{
			/*
			sin(-1*x)=-1*sin(x)
			sinh(-1*x)=-1*sinh(x)
			tan(-1*x)=-1*tan(x)
			*/
		case FUNC_sin:
		case FUNC_tan:
		case FUNC_sinh:
		{
			bool Neg = false;
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					Neg = !Neg;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			if (Neg)pNode = Const(-1) Mul pNode;
			break;
		}
		/*
		cos(-1*x)=cos(x)
		cosh(-1*x)=cosh(x)
		*/
		case FUNC_cos:
		case FUNC_cosh:
		{
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			break;
		}
		/*
		pow(x,-1*y)=1/x^y
		exp(-1*x)=1/exp(x)
		*/
		default:break;
		}
		break;
	}
	default:break;
	}
	return Changed;
}

/**
 * @brief Handles top-level negation expressions
 * @param pNode[in,out] Root node of the entire expression
 * @return bool True if top-level negation was optimized
 *
 * - Converts -(a+b) -> -a-b style distributions
 * - Eliminates unnecessary top-level negative signs
 * - Ensures negation only applies to atomic terms
 */
bool Simplify_TopNeg(ExprNode*& pNode)
{
	bool Changed = false;
	if (pNode->V == MUL)
	{
		bool Neg = false;
		bool FirstNeg = false;
		TraverseType(pNode, MUL, p)
		{
			if (p->V.Ty == Token::Int && p->V.ID == -1)
			{
				if (_I)
				{
					p->V.ID = 1;
					Neg = !Neg;
					Changed = true;
				}
				else FirstNeg = true;
			}
		}
		if (Neg)
		{
			//-1*(-1)*a=a
			if (FirstNeg)TraverseSeries[0]->V.ID = 1;
			//a*(-b)=-1*a*b
			else pNode = Const(-1) Mul pNode;
		}
	}
	return Changed;
}

/**
 * @brief Evaluates constant subexpressions
 * @param pNode[in,out] Root node of the expression subtree
 * @return bool True if any constant folding occurred
 *
 * - Computes arithmetic results for constant operations
 * - Reduces 2+3 -> 5 type evaluations
 * - Handles power-of-constant computations
 */
bool Simplify_FoldConst(ExprNode*& pNode)
{
	if (!pNode)return false;
	//printf("\nFoldConst: "); pNode->PrintTree();
	bool Changed = false;
	Changed |= Simplify_FoldConst(pNode->L());
	Changed |= Simplify_FoldConst(pNode->R());
	if (pNode->V.Ty == Token::Operator)
	{
		if (pNode->Ty0() == Token::Int && pNode->Ty1() == Token::Int)
		{
			switch (pNode->V.ID)
			{
			case '+':ReplaceInt(pNode->L()->V.ID + pNode->R()->V.ID, pNode); Changed = true; break;
			case '-':ReplaceInt(pNode->L()->V.ID - pNode->R()->V.ID, pNode); Changed = true; break;
			case '*':Changed |= RotateCoefficient(pNode); break;
			case '/':
				if (pNode->L()->V.ID && pNode->R()->V.ID)
				{
					auto G = std::gcd(pNode->L()->V.ID, pNode->R()->V.ID);
					pNode->L()->V.ID /= G;
					pNode->R()->V.ID /= G;
				}
				break;
			case '^':
				if (pNode->ID1() > 0)ReplaceInt(int(pow(pNode->ID0(), pNode->ID1())), pNode);
				else if (pNode->ID1() == 0)ReplaceInt(1, pNode);//Filter generated x^0 after Simplify_01
				else
				{
					ReplaceInt(int(pow(pNode->ID0(), -pNode->ID1())), pNode);
					pNode = Const(1) Div pNode;
				}
				Changed = true; break;
			default:break;
			}
		}
		//pow won't appear here so we don't need to consider it
	}
	return Changed;
}


bool Simplify_MonomialImpl(ExprNode*& pNode)
{
	bool Changed = false;
	Changed |= Simplify_MergePower(pNode);
	Changed |= Simplify_All01(pNode);
	Changed |= Simplify_Neg(pNode);
	Changed |= Simplify_TopNeg(pNode);
	return Changed;
}

/**
 * @brief Phase I monomial simplification (structural)
 * @param pNode[in,out] Monomial expression root node
 * @return bool True if monomial structure was modified
 *
 * - Merges identical factors in products
 * - Eliminates 1 coefficients
 * - Orders factors in canonical sequence
 */
bool Simplify_Monomial_I(ExprNode*& pNode)
{
	bool Simplify_Polynomial(ExprNode * &pNode);
	if (!pNode)return false;
	bool Changed = false;
	if (pNode->V == ADD)
	{
		Changed |= Simplify_Polynomial(pNode);
	}
	else
	{
		if (pNode->L())Changed |= Simplify_MonomialImpl(pNode->L());
		if (pNode->R())Changed |= Simplify_MonomialImpl(pNode->R());
		Changed |= Simplify_Monomial_I(pNode->L());
		Changed |= Simplify_Monomial_I(pNode->R());
	}
	return Changed;
}

/**
 * @brief Phase II monomial simplification (coefficient)
 * @param pNode[in,out] Monomial expression root node
 * @return pair<bool, Fraction> (modified, coefficient)
 *
 * - Extracts numerical coefficient from monomial
 * - Normalizes coefficient position (always leftmost)
 * - Returns extracted coefficient and modification status
 */
std::pair<bool, Fraction> Simplify_Monomial_II(ExprNode*& pNode)
{
	auto&& [Changed, F] = ExtractCoefficient(pNode);
	Changed |= Simplify_MonomialImpl(pNode);
	return { Changed,F };
}

/**
 * @brief Phase III monomial simplification (final)
 * @param pNode[in,out] Monomial expression root node
 * @return bool True if final optimizations applied
 *
 * - Applies sign normalization
 * - Removes redundant exponent of 1
 * - Ensures consistent monomial representation
 */
bool Simplify_Monomial_III(ExprNode*& pNode)
{
	bool Changed = false;
	Changed |= Simplify_All01(pNode);
	Changed |= Simplify_Neg(pNode);
	Changed |= Simplify_TopNeg(pNode);
	return Changed;
}

/**
 * @brief Simplifies polynomial expressions
 * @param pNode[in,out] Root node of polynomial expression
 * @return bool True if polynomial terms were combined
 *
 * - Combines like terms in additive expressions
 * - Factors common subexpressions across terms
 * - Orders terms in canonical sequence
 */
bool Simplify_Polynomial(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;

	//STAGE I
	TraverseTypeLocal(C, pNode, ADD, p)
	{
		if (!p->IsConst()) Changed |= Simplify_Monomial_I(p);
	}

	//STAGE II
	std::vector<Fraction> Coefficients;
	TraverseTypeLocal(D, pNode, ADD, q)
	{
		if (q->IsConst())
		{
			Coefficients.emplace_back(1);
			continue;
		}
		auto q1 = CreateNode(); q1->Copy(q);
		auto&& [C, F] = Simplify_Monomial_II(q1);
		Changed |= C;
		q->Copy(q1); ReleaseNode(q1);
		if constexpr (EnableDebugSimplifyII) { printf("PushCoeff: "); F.ToNode()->PrintTree(); putchar('\n'); }
		Coefficients.push_back(F);
	}

	//STAGE III
	{
		std::map<ExprHash, int>AMP;
		for (size_t I = 0; I < D.size(); I++)
		{
			if (D[I]->IsConst())continue;
			auto H = D[I]->Hash();
			//exp(-y)*cos(x)^2-exp(-y)*sin(x)^2
			if constexpr (EnableDebugSimplifyII) { printf("Stage3 Hash %016llX Tree:", H); D[I]->PrintTree(); putchar('\n'); }
			auto it = AMP.find(H);
			if (it == AMP.end())AMP[H] = (int)I;
			else
			{
				Coefficients[it->second] += Coefficients[I];
				Coefficients[I] = Fraction(0);
				ReleaseTree(D[I]->L());
				ReleaseTree(D[I]->R());
				ClearNode(D[I]);
				D[I]->V = Token(int(0));
				Changed = true;
			}
		}
	}

	//STAGE IV
	{
		//x*y+x*z=x*(y+z)
		std::set<ExprHash>TriedFactors;
		std::vector<std::map<ExprHash, ExprNode**>> Factors;
		std::map<ExprHash, CommonFactor> Common;
		Factors.resize(D.size());
		for (size_t I = 0; I < D.size(); I++)
			FillFactorMap(D[I], Factors[I]);
		for (size_t I = 0; I < D.size(); I++)
		{
			if (D[I]->IsConst())continue;
			for (size_t J = 0; J < I; J++)
			{
				if (D[J]->IsConst())continue;
				bool MergeIJ = false;
				GetCommonFactor(Factors[I], Factors[J], Common);
				if constexpr (EnableDebugSimplifyII) 
				{ 
					printf("Common Factors:\n"); 
					for (auto& [k, v] : Common)
					{
						v.pi->PrintTree(); 
						putchar('\n');
					}
				}
				if (!Common.empty())
				{
					for (auto& [K, V] : Common)
					{
						*V.ppi = Const(1);
						*V.ppj = Const(1);
					}
					auto E1 = D[I]->Duplicate();
					auto E2 = D[J]->Duplicate();
					for (auto& [K, V] : Common)
					{
						ReleaseNode(*V.ppi); *V.ppi = V.pi;
						ReleaseNode(*V.ppj); *V.ppj = V.pj;
					}
					auto F = DuplicateFactor(Common);

					auto K = F Mul((Coefficients[I].ToNode() Mul E1) Add(Coefficients[J].ToNode() Mul E2));
					if constexpr (EnableDebugSimplifyII) { printf("OrigK: "); K->PrintTree(); putchar('\n'); }
					Simplify(K);
					if constexpr (EnableDebugSimplifyII) { printf("SimpK: "); K->PrintTree(); putchar('\n'); }
					ReleaseTree(D[I]->L());
					ReleaseTree(D[I]->R());
					ReleaseTree(D[J]->L());
					ReleaseTree(D[J]->R());
					D[I]->L() = K->L();
					D[I]->R() = K->R();
					D[I]->V = K->V;
					ReleaseNode(K);
					ClearNode(D[J]);
					D[J]->V = Token(int(0));
					Coefficients[I] = Coefficients[J] = Fraction(1);
				}
			}
		}
	}

	//STAGE V
	for (size_t I = 0; I < D.size(); I++)
	{
		if (D[I]->IsConst())continue;
		auto U = CreateNode(); U->Copy(D[I]);
		if constexpr (EnableDebugSimplifyII) { printf("Before: "); U->PrintTree(); putchar('\n'); }
		if constexpr (EnableDebugSimplifyII) { printf("Coeff: "); Coefficients[I].ToNode()->PrintTree(); putchar('\n'); }
		auto V = Coefficients[I].ToNode() Mul U;
		Simplify_Monomial_III(V);
		if constexpr (EnableDebugSimplifyII) { printf("After: "); V->PrintTree(); putchar('\n'); }
		D[I]->Copy(V);
	}
	return Changed;
}

/**
 * @brief Final normalization of negative signs in the expression
 * @param pNode[in,out] Root node of the expression tree
 *
 * - Converts nested negations to positive terms (-(-x) -> x)
 * - Moves negative coefficients to canonical positions
 * - Simplifies multiplication/division with negative signs
 */
bool FinalFold_Neg(ExprNode*& pNode)
{
	if (!pNode)return false;
	bool Changed = false;
	Changed |= FinalFold_Neg(pNode->L());
	Changed |= FinalFold_Neg(pNode->R());
	switch (pNode->V.Ty)
	{
	case Token::Operator:
	{
		//x^((-1)*y)=1/x^y
		if (pNode->V.ID == '^')
		{
			bool Neg = false;
			TraverseType(pNode->R(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					Neg = !Neg;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			if (Neg)pNode = Const(1) Div pNode;
		}
		else if (pNode->V.ID == '+')
		{
			bool NegR = false;
			TraverseType(pNode->R(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					NegR = !NegR;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			bool NegL = false;
			TraverseType(pNode->L(), MUL, q)
			{
				if (q->V.Ty == Token::Int && q->V.ID < 0)
				{
					NegL = !NegL;
					q->V.ID = -q->V.ID;
					Changed = true;
				}
			}
			if (NegL)
			{
				if (NegR)
				{
					//(-1)*a+(-1)*b=(-1)*(a+b)
					pNode = Const(-1) Mul pNode;
				}
				else
				{
					//(-1)*a+b=b-a
					pNode->V.ID = '-';
					std::swap(pNode->L(), pNode->R());
				}
			}
			else if (NegR)
			{
				//a+(-1)*b=a-b
				pNode->V.ID = '-';
			}
		}
		else if (pNode->V.ID == '-')
		{
			bool NegR = false;
			TraverseType(pNode->R(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					NegR = !NegR;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			bool NegL = false;
			TraverseType(pNode->L(), MUL, q)
			{
				if (q->V.Ty == Token::Int && q->V.ID < 0)
				{
					NegL = !NegL;
					q->V.ID = -q->V.ID;
					Changed = true;
				}
			}
			/*
			 a-(-1)*b=a+b
			(-1)*a-b=(-1)*(a+b)
			(-1)*a-(-1)*b=b-a
			*/
			if (NegL)
			{
				if (NegR)std::swap(pNode->L(), pNode->R());
				else
				{
					pNode->V.ID = '+';
					pNode = Const(-1) Mul pNode;
				}
			}
			else if (NegR)pNode->V.ID = '+';
		}
		break;
	}
	case Token::Function:
	{
		switch (pNode->V.ID)
		{
			/*
			sin(-1*x)=-1*sin(x)
			cos(-1*x)=cos(x)
			tan(-1*x)=-1*tan(x)
			*/
		case FUNC_sin:
		case FUNC_tan:
		case FUNC_sinh:
		{
			bool Neg = false;
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					Neg = !Neg;
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			if (Neg)pNode = Const(-1) Mul pNode;
			break;
		}
		/*
		sinh(-1*x)=-1*sinh(x)
		cosh(-1*x)=cosh(x)
		*/
		case FUNC_cos:
		case FUNC_cosh:
		{
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID < 0)
				{
					p->V.ID = -p->V.ID;
					Changed = true;
				}
			}
			break;
		}
		/*
		pow(x,-1*y)=1/x^y
		exp(-1*x)=1/exp(x)
		*/
		case FUNC_exp:
		case FUNC_pow:
		{
			bool Neg = false;
			TraverseType(pNode->L(), MUL, p)
			{
				if (p->V.Ty == Token::Int && p->V.ID == -1)
				{
					Neg = !Neg;
					p->V.ID = 1;
					Changed = true;
				}
			}
			if (Neg)pNode = Const(1) Div pNode;
			break;
		}
		default:break;
		}
		break;
	}
	default:break;
	}
	return Changed;
}

/**
 * @brief Extracts greatest common divisor from monomial terms
 * @param pNode[in,out] Root node of monomial expression
 *
 * - Operates on single-term expressions (e.g., 4x^2*y)
 * - Factorizes numerical coefficients (4x^2*y -> 4*(x^2*y))
 * - Preserves variable structure during GCD extraction
 */
void FinalFold_GCDMono(ExprNode*& pNode)
{
	void FinalFold_GCDPoly(ExprNode * &pNode);
	if (!pNode)return;
	std::vector<ExprNode*> Cont;
	if (pNode->V == ADD)
	{
		FinalFold_GCDPoly(pNode);
	}
	else
	{
		FinalFold_GCDMono(pNode->L());
		FinalFold_GCDMono(pNode->R());
	}
}

/**
 * @brief Merges identical factors in multiplication expressions
 * @param pNode[in,out] Root node of multiplicative expression
 * @param Tg[in,out] Hash map tracking factor occurrences 
 *
 * - Combines identical base factors (x*x -> x^2)
 * - Uses hash matching for rapid duplicate detection
 * - Maintains expression structure integrity
 */
void FinalFold_MergeSame(ExprNode*& pNode, std::map<ExprHash, ExprNode**>& Tg)
{
	//y^x*z^x=(y*z)^x
	if (pNode->V == MUL)
	{
		FinalFold_MergeSame(pNode->L(), Tg);
		FinalFold_MergeSame(pNode->R(), Tg);
	}
	else if (pNode->V == POW)
	{
		auto H = pNode->R()->Hash();
		auto it = Tg.find(H);
		if (it == Tg.end())Tg[H] = &pNode;
		else
		{
			auto T = *(it->second);//y^x z^x
			T->L() = T->L() Mul pNode->L();//y^x z^x -> //(y*z)^x z^x
			pNode->L() = Const(1);//(y*z)^x 1^x
		}
	}
}

/**
 * @brief Optimizes power expressions through exponent merging
 * @param pNode[in,out] Root node containing power terms
 *
 * - Applies power rules (x^a * x^b -> x^(a+b))
 * - Handles nested power structures ((x^a)^b -> x^(a*b))
 * - Normalizes power distribution over multiplication
 */
void FinalFold_MergePower(ExprNode* pNode)
{
	std::map<ExprHash, ExprNode**> T;
	auto K = CreateNode(); K->Copy(pNode);
	FinalFold_MergeSame(K, T);
	pNode->Copy(K);
	ReleaseNode(K);
}

/**
 * @brief Factors out GCD from polynomial expressions
 * @param pNode[in,out] Root node of polynomial expression
 *
 * - Analyzes all terms in additive expressions
 * - Extracts numerical and variable common factors
 * - Restructures expression with factored-out GCD
 */
void FinalFold_GCDPoly(ExprNode*& pNode)
{
	if (!pNode)return;
	if (pNode->V == Token(int(0)))return;
	TraverseTypeLocal(C, pNode, ADD, p)
		FinalFold_GCDMono(p);
	std::vector<Fraction> Coefficients;
	TraverseTypeLocal(D, pNode, ADD, q)
	{
		Coefficients.push_back(ExtractCoefficient(q).second);
		FinalFold_MergePower(q);
		if constexpr (EnableDebugSimplifyII) { printf("Coefficient %d/%d\n", Coefficients.back().N, Coefficients.back().D); }
	}
	auto Tg = ExtractGCD(Coefficients.begin(), Coefficients.end());
	for (size_t I = 0; I < D.size(); I++)
	{
		auto U = CreateNode();
		U->Copy(D[I]);
		auto F = Tg == 0 ? Fraction(0) : Coefficients[I] / Tg;
		if (Tg == 0)
		{
			if (!DividedbyZero)puts("Runtime Error: Divided by 0");
			DividedbyZero = true;
			return;
		}
		auto V = F.ToNode() Mul U;
		D[I]->Copy(V);
	}
	pNode = Tg.ToNode() Mul pNode;
}

/**
 * @brief Final simplification pass with GCD factoring and negation cleanup
 * @param pNode[in,out] Root node of the fully processed expression
 */
void FinalFold(ExprNode*& pNode)
{
	if (!pNode)return;
	FinalFold_GCDPoly(pNode);
	FinalFold_Neg(pNode);
	Simplify_TopNeg(pNode);
	bool Changed;

	// Repeat until no more constant folding possible
	do
	{
		Changed = false;
		Changed |= Simplify_FoldConst(pNode);//Combine generated Const above
		Changed |= Simplify_All01(pNode);//Remove generated 01 above
	} while (Changed);
}

/**
 * @brief Simplifies the expression tree rooted at the given node.
 *
 * @param pNode The root node of the subtree to be simplified.
 */
void Simplify(ExprNode*& pNode)
{
	std::set<ExprHash> Occurred;
	if constexpr (EnableDebugSimplifyI) { printf("\nInitial: "); pNode->PrintTree(); }
	do
	{
		Simplify_All01(pNode);         
		if constexpr (EnableDebugSimplifyI) { printf("\nAll01: "); pNode->PrintTree(); putchar('\n'); }
		Simplify_Neg(pNode);           
		if constexpr (EnableDebugSimplifyI) { printf("\nNeg: "); pNode->PrintTree(); putchar('\n'); }
		Simplify_TopNeg(pNode);        
		if constexpr (EnableDebugSimplifyI) { printf("\nTNeg: "); pNode->PrintTree(); putchar('\n');}
		Simplify_SpecialFuncs(pNode);  
		if constexpr (EnableDebugSimplifyI) { printf("\nSpFn: "); pNode->PrintTree(); putchar('\n'); }
		Simplify_Polynomial(pNode);    
		if constexpr (EnableDebugSimplifyI) { printf("\nPoly: "); pNode->PrintTree(); putchar('\n'); }
		Simplify_FoldConst(pNode);     
		if constexpr (EnableDebugSimplifyI) { printf("\nFold: "); pNode->PrintTree(); putchar('\n'); }
	} // Loop until hash stabilizes
	while (Occurred.insert(pNode->Hash()).second);

	FinalFold(pNode);

	if constexpr (EnableDebugSimplifyI) { printf("\nFinal: "); pNode->PrintTree(); putchar('\n'); }
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------
//--------------------------MAIN FUNCTION--------------------------
//-----------------------------------------------------------------
//-----------------------------------------------------------------

/**
 * @brief Main entry point for expression processing and differentiation
 *
 * - Continuously processes mathematical expressions from standard input
 * - Performs tokenization, parsing, and partial derivative calculations
 * - Outputs derivatives for all variables in the expression
 * @return int Always returns 0 for standard program termination
 */
int main()
{
	while (1)
	{
		// -purpose: Manages resource cleanup between parsing rounds
		// -usage: Constructor resets global state, destructor releases memory
		RoundGuard G;

		// Read mathematical expression from standard input
		std::getline(std::cin, Expression);

		// -purpose: Stores tokenized components of the input expression
	    // -usage: Feed to parser for expression tree construction
		GenerateTokens(Expression, Tokens);
		{
			// -purpose: Main expression container with parsing capabilities
			// -usage: Constructs expression tree from tokens
			Expr Original(Tokens);

			// Skip invalid expressions and division-by-zero cases
			if (FailedToParse || DividedbyZero)continue;
			if constexpr (EnableDebugSimplifyI) { Original.Print(); }

			// Calculate and print partial derivatives for each variable
			for (auto [Name, ID] : VarMap)
			{
				// -purpose: Stores derivative expression for current variable
				// -usage: Automatically simplifies during construction
				Expr Partial(Original, ID);
				if (DividedbyZero)continue;
				printf("%s: ", Name.c_str());
				Partial.Print();
			}
		}
	}
	return 0;
}
