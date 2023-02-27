#include <iostream>
#include <vector>
#include<string>

//This code can be used to print out the value category of an expression.
template<typename T>
struct value_category {
    static constexpr auto value = "prvalue";
};
template<typename T>
struct value_category<T&> {
    static constexpr auto value = "lvalue";
};
template<typename T>
struct value_category<T&&> {
    static constexpr auto value = "xvalue";
};
#define VALUE_CATEGORY(expr) value_category<decltype((expr))>::value



//--------------------------------------------------
//FORWARDING
//--------------------------------------------------

//If you have been using function templates a lot, you may have come across some strange behavior.
//Given a function template with an rvalue type parameter, like this one:

template<typename T>
void exampleFunc1(T&& val){std::cout << "func1 called\n";}

static int a = 5;

//You could call it like so:
//    exampleFunc1(a);

//This is because it is a forwarding reference (sometimes called a universal reference), not an rvalue reference.
//A forwarding reference is declared when a function parameter in a function template is declared
//as a cv-unqualified rvalue reference to a template type parameter of the function template.
//It's also declared when you write "auto&&", except where auto is deduced by an initializer list.
//Here are a few examples to help explain when a reference is a forwarding reference vs an rvalue reference:

template<typename T>
void exampleFunc2(const T&& val); //val is not a forwarding reference, since it's cv-qualified

template<typename T>
class A{
    template<typename T1>
    void foo(T&& x, T1&& y); //x isn't a forwarding reference because the type parameter is from the class, not the function.
    //However, y is a forwarding reference.
    
};

//A forwarding reference accepts expressions of any value category (lvalue, prvalue, xvalue), much like a const T&.
//However, when the template parameter is deduced it also preserves the value category of the expression.
//If the expression is an lvalue or lvalue reference, the forwarding reference will be an lvalue reference.
//Otherwise, it's an rvalue reference.
//It also preserves the cv-qualifiers of the expression. Const expressions will remain const, volatile will remain volatile.
//Note that this only happens when the type is deduced: if you explicitly supply it when calling the function,
//it won't preserve the value category.


//This is used in conjunction with the std::forward function to enable perfect forwarding.
//std::forward is a function template defined in the utility header.
//It takes a variable of any type, and just returns it.
//If it's a non-reference type or an rvalue reference, it returns an rvalue reference (as though you had used std::move)
//Otherwise, it returns an lvalue reference.
//The type of the template parameter can't be deduced, so you need to give it explicitly
//If T is the forwarding reference in the function with variable name val, then std::forward<T>(val)
//is equivalent to static_cast<T&&>(val).


int foo(int&){std::cout << "lvalue\n"; return 0;}
int foo(int&&){std::cout << "rvalue\n"; return 0;}

template<typename T>
void exampleFunc3(T&& val){bar(std::forward<T>(val));}

//Given the above functions, if you write
//    exampleFunc3(5);
//It will call the rvalue foo, but if you write
//    exampleFunc3(a);
//It will call the lvalue foo.


//That's basically all there is to forwarding, and it's primary use.
//It's just there so that you can optimize move semantics.
//For example, in std::vector and other STL containers there is a member function template called emplace_back(),
//Which takes in a parameter pack and uses the arguments to construct the type the container holds in place.
//If you wrote this:
//    std::vector<std::string> v1;
//    v1.emplace_back(5, 'h');

//The emplace_back call the std::string constructor using the arguments supplied (like std::string(5, 'h')).
//Suppose that forwarding references weren't a thing, and the && always referred to an rvalue reference.
//Then you would need to make two different emplace_back functions in order to make use of std::move.
//Furthermore, if even one of the arguments were an lvalue, then it would have to use the lvalue version of emplace_back.
//However, with forwarding you ignore all that.
//Look at the function below:


template<typename ...Ts>
void exampleFunc4(Ts&&... vals){
    auto a = {foo(std::forward<Ts>(vals))...};
//The auto a is just there to allow for the parameter pack expansion, you can ignore it.
}

//If you called it like
//    exampleFunc4(5, a, 5, a, 5);
//Then it will call the rvalue, lvalue, rvalue, lvalue, and rvalue foo functions.
//Without forwarding, it would have to be all rvalues or lvalues.



int main(){
    
    exampleFunc1(a);
    
    exampleFunc4(5, a, 5, a, 5);
    
    std::vector<std::string> v1;
    v1.emplace_back(5, 'h'); //Uses std::string(5,'h') to construct a new std::string at the end of the vector.
    
    
    return 0;
}
