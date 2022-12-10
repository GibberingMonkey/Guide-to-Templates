
#include <iostream>
#include <array>
#include <vector>

//Function templates as member functions

//--------------------------------------------------
//FUNCTION TEMPLATES
//--------------------------------------------------


//As mentioned in the intro to templates section, a function template can be defined like so:
template<typename T1, class T2> void exampleFunc1(T1 variable1, T2 variable2){
    std::cout << variable1 << " " << typeid(T1).name() << " ";
    std::cout << variable2 << " " << typeid(T2).name() << std::endl;
}


//Like regular functions, function templates can be overloaded.
//Function templates are distinct from non-templated functions. If you defined this function:

void exampleFunc1(int variable1, int variable2){
    std::cout << "Non-templated function called\n";
}

//The code exampleFunc1(5, 3); would still compile even though the templated instantiation would look exactly the same as the non-templated function.
//The code would call the non-templated function: the compiler first looks for a valid function call among non-templated functions, then function templates.
//If you wanted to ensure that you called the function template, you should provide the parameters explicitly, like so:
//exampleFunc1<int, int>(5,3);
//Or specify that you're using a template with an empty parameter list:
//exampleFunc1<>(5,3);



//When overloading function templates, the return type is considered to be part of the function signature.
//For example, your code will still compile if you define these two functions:

template<int I, int J>
std::array<int, I+J> exampleFunc2(std::array<int, I>, std::array<int, J>){return std::array<int, I+J>{2};} //overload 1

template<int I, int J>
std::array<int, I-J> exampleFunc2(std::array<int, I>, std::array<int, J>){return std::array<int, I+J>{1};} //overload 2

//However, in order to call the function you need to declare which return type you're using (like by assigning it to a variable) or by making
//sure one overload would not work (see main() for a full example).


//Two expressions involving template parameters are equivalent if they are the same, except for the names of the parameters (which may or may not be different).
//For example, the commented out function definition is equivalent to overload 1 of exampleFunc2, and would constitute a redefinition of that overload.

//template<int K, int L>
//std::array<int, K+L> exampleFunc2(std::array<int, K>, std::array<int, L>){return std::array<int, K+L>{2};} // same as overload 1


//Two function templates are equivalent if:
//    They are in the same scope and have the same name
//    The expressions involving template parameters in the function declaration are equivalent.
//    And they have equivalent parameter lists, meaning that for each corresponding parameter pair:
//        The parameters are the same kind (type, non-type, etc.)
//        If non-type, their types are equivalent
//        If templates, their parameter lists are equivalent.


//Two expressions are functionally equivalent if they aren't equivalent, but they will always evaluate to the same expression.
//Two function templates are functionally equivalent if they are equivalent, except that at least one expression involving template parameters
//in the function declaration is functionally equivalent. For example, the following two function templates are functionally equivalent:

template<int A>
void exampleFunc3(std::array<int, A+10>){}

template<int A>
void exampleFunc3(std::array<int, A+5*2>){}

//If your code contains two function declarations that are functionally equivalent (but not equivalent), then it is ill-formed, no diagnostic required.
//This means that your code does not follow the standard, but it may still compile. Furthermore, the compiler does not need to issue any sort of warning.
//In other words, it is undefined behavior and should be avoided.
//Functionally equivalent function templates don't necessarily produce an error because it can be incredibly hard to figure out when
//they are functionally equivalent, if not practically impossible in some cases, so it's simpler to say that it's undefined behavior and leave it at that.

//Other than this, function templates work the same as normal functions.




//--------------------------------------------------
//ALIAS TEMPLATES AND VARIABLE TEMPLATES
//--------------------------------------------------

//These two types of templates are pretty simple

//A type alias is defined like this:
using Type = int;

//The keyword using indicates that you're using a type alias. It makes the name on the left side of the = a synonym of the type-id (such as std::vector<int>, float, char*) on the right.
//In other words, you can now do this in the program:

Type a = 5;

//An alias template is the pretty much the same exact thing, except you can now use template parameters:
template<typename T>
using vec = std::vector<T>;

//Alias templates are really only used to make aliases of types involving other templates.
//The only thing to watch out for is that alias templates don't allow the compiler to implicitly deduce template parameters.
//The following code would give an error:

//vec v1 = std::vector<int>{1, 2, 3, 4};

//Even though the compiler should be able to deduce that the T in vec<T> would be an int.




//Variable templates are defined by placing a template declaration (the keyword template followed by a parameter list) before a variable declaration.
//For example:

template<typename T>
constexpr T pi = T(3.1415926535897932385L);


//There isn't anything special about them, other than the fact that a variable template can only be a class member if it is static.




int main(){
    
//    This function call works because the second overload can't fit this.
//    Since I and J are integers, if the compiler tried to call the second overload the return value would be std::array<int, -1>{1}.
//    Since narrowing conversions aren't allowed with non-type parameters, that is not a valid instantiation of std::array
//    so the second overload won't work, meaning that the compiler then just calls the first overload.
    exampleFunc2<1, 2>(std::array<int, 1> {2}, std::array<int, 2> {1});
    
    
    
    
    
    return 0;
}
