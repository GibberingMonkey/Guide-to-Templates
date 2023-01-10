#include <vector>
#include <array>
#include <iostream>

template<typename... Args, template<typename> class ...Args1, typename... Args2>
void exampleFunc1();

template<typename... Args>
void exampleFunc2(Args ...args);


//The above function contains a template parameter pack and a function parameter pack.
//A template parameter pack (a parameter pack within a template parameter list) is a parameter that can accept
//any number of parameters, including 0.
//It can accept non-type, type, or template parameters.
//To declare a parameter pack, you write the template parameter as normal but put an ellipsis prior to where the name of the
//parameter would normally go (though parameter packs don't need a name).
//A template which has a parameter pack is called a variadic template.

//A function parameter pack (a parameter pack within a function parameter list) is a parameter that accepts any number,
//of arguements, including 0 arguments.
//To declare a function parameter pack, you make the type a template parameter pack and put an ellipsis before the name (which is optional).
//The function parameter pack then takes in the same number of arguments that are supplied to the template parameter pack.
//For example, this is not a valid function call:
//    exampleFunc2<int, int, int>(1, 1);

//Whereas these function calls would be:
//    exampleFunc2<int, int, int>(1, 1, 2);
//    exampleFunc2(1, 1, 2); //Here, the compiler deduces that the template parameter pack is int, int, int.
//    exampleFunc2(1, 1);
//    exampleFunc2<int, int>(1, 1);

//In function templates, a template parameter that appears after a template parameter pack must have a default argument,
//be deducible from the function signature, or be another parameter pack.
//If two parameter packs are in a row, they must be of different parameter types (type, template, or non-type).
//In other templates, there can only be one parameter pack and it must be the last parameter.
//If you want to find out the length of a parameter pack, you can use the sizeof... operator.
//Given a parameter pack called Args of size N, sizeof... (Args) will return N.
//This operator is constexpr, so the value it returns is evaluated at compile time.


//In order to actually use parameter packs, you need to do pack expansion.
//When a pattern includes the name of a parameter pack and is followed by an ellipsis, it is expanded into
//a number of instantiations of the pattern where the name of the parameter pack is replaced by
//each element of the pack, in order.
//This isn't the most clear explanation, so here are a few examples of pack expansions:

template<typename... Args>
void exampleFunc2(Args ...args){
    std::make_tuple(args...); //Expansion 1
    std::vector<size_t> v1 = {sizeof(args)...}; //Expansion 2
    std::tuple<std::vector<Args>...> t1; //Expansion 3
    
}

//Suppose the function is called like this:
//    exampleFunc2(5, 2.0, 'c');

//Expansion 1 uses the most basic form of pack expansion.
//The pattern in the pack expansion is simply args, which is expanded to a comma separated list of all its elements.
//The "args..." gets replaced by "5, 2.0, 'c'" in expansion 1.

//Expansion 2 expands the function parameter pack "args" with a pattern. A pattern can be thought of like a function.
//If you apply an operation to the parameter pack, then when it is expanded that operation is applied to each of those elements
//In expansion 2, the function can be thought of as f(x) = sizeof(x).
//The "sizeof(args)..." gets replaced by "sizeof(5), sizeof(2.0), sizeof('c')" in expansion 2.

//Expansion 3 also uses a pattern, but this time expands the template parameter pack "Args".
//This time, the pattern is "std::vector<Args>", so it gets expanded to "std::vector<int>, std::vector<double>, std::vector<char>".

//In the pack expansion "&args...", the pattern is "&args". In "func(args)...", the pattern is "func(args)"
//If pattern contains two parameter packs, they are expanded at the same time and must be the same length.
//If there is an expansion nested within another expansion, the innermost expansion occurs first.



//Fold expressions are essentially special types of patterns using binary operators.
//The function below uses all four types of fold expressions:
//You can use any type of binary operator instead of the + operator

template<int ... Vals>
std::array<int, 4> exampleFunc3(){
    return std::array<int, 4> {
        (Vals + ...), //unary right fold
        (... + Vals), //unary left fold
        (Vals + ... + 0), //binary right fold
        ( (0 + 0) + ... + Vals) //binary left fold
    };
}


//The unary folds only have a parameter pack and ellipsis separated by an operator,
//and are expanded so that the operator is placed between the elements of the parameter pack.
//Note that the opening and closing parentheses are necessary.
//The difference between the left and right folds is just the order in which the operators are evaluated.

//If Vals has elements e_1, ..., e_{n-1}, e_n, then Vals + ... expands to (e1+(...+(e_{n-1}+e_n))) and
//(... + Vals) expands to (((e_1+ e_2)+...)+e_n).

//The binary folds add an additional expression to the expansion.
//This expression cannot contain an unexpanded parameter pack, nor can it use an operator
//with a lower precendence than a cast at the top level.
//Basically, this means that the expression cannot have something like a + in it ((0 + 0 + ... + Vals) wouldn't work),
//but something like the subscript operator would be fine.
//However, you can you can just stick a pair of parentheses around the expression and it be fine, since only the top-level operator matters.

//The binary folds expand in the same way, except that the additional expression is placed in the innermost pair of parentheses.
//For example, using the elements of Vals from above, (Vals + ... + 0) expands to (e1+(...+(e_n+0)))


//Suppose you called the function like this:
//    exampleFunc3<>();
//That line would produce a build error, because the unary folds would be expanding an empty parameter pack.
//Only three operators can be used in a unary fold expression with a pack expansion of length 0:
//logical OR (||), logical AND (&&), and the comma operator (,).
//For ||, it evaluates to false. For &&, it evaluates to true. For the , operator it evaluates to void().
//If this behavior is undesirable, you should use the binary fold expressions instead, and make the additional expression your desired default.


int main(){
    
    auto a = exampleFunc3<1, 2, 3, 4>();
    
    for(int element : a){std::cout << element << std::endl;}
    
    
    return 0;
}
