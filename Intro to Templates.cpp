
//NEED TYPE DEDUCTION and DEDUCTION GUIDES, TEMPLATE METAPROGRAMMING, SEPARATING DECLARATIONS AND DEFINITIONS, STD::ENABLE_IF, FORWARDING, EXTERN LINKAGE and EXPLICIT/IMPLICIT INSTANTIATION, PARTIAL ORDERING, CRTP (Curiously recurring template pattern), EXPRESSION TEMPLATES, PARAMETER PACKS

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>



//--------------------------------------------------
//INTRO TO TEMPLATES
//--------------------------------------------------

//Templates are how C++ implements generic programming.
//There are four kinds of templates: class templates, function templates, variable templates, and alias templates.
//Templates aren't actual classes, functions, etc. themselves, but rather a guide for the compiler on how to create those things based on certain parameters that are provided when the template is instantiated.
//Because they're generating classes and functions, templates are fully evaluated at compile time. This means that they don't have any runtime cost.
//Furthermore, templates are compiled differently as well: they use two phase lookup.
//In the first phase, the template is checked for syntax. In the second phase, the types supplied to the template are substituted into the template and it's checked again (though some compilers skip the first phase entirely).
//Notably, this second phase only occurs when a template is instantiated: if you never use the template, the second phase will never happen.
//This can lead to compile time errors that aren't always present when you compile your code. For example:

template<typename>
struct A{
    A(int){}
};

template<typename T>
struct B: public A<T>
{
    B(){}
};

//The code above defines two class templates, A and B, such that B is derived from A.
//The default constructor of A is implicitly deleted, so the constructor for B must explicitly construct A.
//It would be like doing this:

//struct RegA{
//    RegA(int){}
//};
//
//struct RegB: public RegA
//{
//    RegB(){}
//};

//While the commented code would produce an error, the templated code does not until you actually try to create an instance of B.


//--------------------------------------------------
//TEMPLATE PARAMETERS
//--------------------------------------------------


//A function template can be defined like this:
template<typename T1, class T2>
void exampleFunc1(T1 variable1, T2 variable2){
	std::cout << variable1 << " " << typeid(T1).name() << " ";
	std::cout << variable2 << " " << typeid(T2).name() << std::endl;
}

//The template keyword states that you're using a template, which is then followed by a parameter list enclosed by < and >.
//A parameter list a comma separated list which contains four kinds of parameters: a non-type parameter, a type parameter, a template parameter, and a parameter pack.
//Each of these parameters are evaluated at compile time.




//The above function contains a type parameter.
//A type parameter is declared by writing a type parameter key (which can be either the keyword "typename" or "class", there's no difference between the two in this context), followed by a name (which is optional).
//However, I'm unable to think of a situation where you wouldn't want to name the type.
//When you use a type parameter, it means that you need to supply a type to the function when you call it, either implicitly or explicitly.
//The name of the type parameter is then used as an alias of the type (in the above function, equivalent to using T = name_of_type;).
//For example, if the type you provide is int, within the function you could use T anywhere and it would be the same as using int.
//The type can also be other templated objects, like std::vector. However, you'd need to specify the types of that other template (the type would need to be like std::vector<int> instead of std::vector).
//You should use type parameters when you want a function to mostly do the same thing but take in different types, like some sort of print function.

//There are two ways to call a function template with a type parameter.
//You could provide the types enclosed by <> after the name of the function, like this:
//	exampleFunc1<int, double>(5, 7.7);
//This is very similar to how you typically create an instance of std::vector, and is called explicit instantiation

//Alternatively, you could call it like you would any other function and let the compiler deduce the types.
//If you call the function like this:
//	exampleFunc1(3.0f, "Hello");
//The compiler automatically tries to deduce the types based on the variables in the function call.
//In this case, it interprets T1 to be a float and T2 to be a char*
//Type deduction will also be its own section.

//You can combine these two methods:
//	exampleFunc1<int>(5, 7); is a valid function call.
//However, the explicit type declarations will always be in the order of the parameter list in the function declaration.



template<int N>
void exampleFunc2(){
	std::cout << N << std::endl;
}


//The above function contains a non-type parameter. Such a parameter has a known type (which includes pointers and references),
//but the actual value of the variable needs to be supplied.
//The type of the non-type parameter can depend on another template parameter, like so:
template<typename T, T N>
void exampleFunc3(){
    std::cout << N << std::endl;
}


//The non-type parameter cannot be any type however. In fact, they're very limited in the types you can use.
//Non-type parameters can only be integral types, enums, non-void pointers and references, and std::nullptr_t (the type of nullptr).
//You can also have function types and arrays, but those are automatically replaced by pointers.
//However, in practice this means you're typically constricted to just integers, enums, and function pointers because of how the pointers work.
//In order for you to use pointers or references, the address of the thing you're referencing must be known at compile time.
//For this reason, any object you pass into a pointer or reference non-type parameter must be in static memory.
//You may wonder what the point of this is, since you could just as easily write a function like below which would do the same thing:

void normalExampleFunc(int N){
	std::cout << N << std::endl;
}

//The main difference is that when you specify a non-type parameter, it must be known at compile time.
//This lets you do things that you wouldn't normally be able to do with functions.
//For example, in exampleFunc2 you could create an array of size N, which you can't do with exampleFunc3 (unless your compiler supports variable length arrays)
//because you wouldn't be able to guarantee that the function parameter would be known at compile time.
//Furthermore, non-type parameters have stricter rules for allowing implicit conversion.
//The only type of conversions allowed when supplying non-type parameters are converted constant conversions, which are these conversions:
//            User defined conversions which are constexpr
//            lvalue to rvalue and qualification (like int to const int) conversions
//            Integral promotions (going from a smaller integral type to a larger one, like int to long int)
//            array to pointer, function to pointer, and converting pointers to and from nullptr
//            function pointer conversions (pointer to noexcept function to pointer to function)
//            Non-narrowing integral conversions (you can't lose precision in a conversion between integral types)
//            And if any reference binding takes place, it doesn't create a temporary object.
//
//Basically, if the conversion can happen at compile time and (if the conversion not user-defined) doesn't lead to a loss in data, it's fine.



//For example, if you call the function like this:
//	exampleFunc2<5.0>();
//It will produce a build error, since 5.0 is a double instead of an int.
//However, you would be able to call the other function with 5.0, and it would convert the argument to an int.



template<template<typename, typename> class T>
void exampleFunc4();

//The above function declaration contains a template template parameter. This parameter can either be a class template or an alias template, but not a function or variable template.
//It's declared as you would normally declare a type parameter, except that it has another template declaration before it.
//The inner template declaration needs to be able to have the same exact template parameters (in number and type) as the template you supply to it.
//For example, to use std::vector you need two type parameters, even though you'd normally instantiate it without providing an allocater.
//This parameter is the actual template (as in std::vector, not std::vector<int>), so whenever you use it in the function you need to supply its arguments.


//While the parameters of the template template parameter can be given names, you can't actually use those names within the the function template. For example, this function:

//template<template<typename A> class T>
//void exampleFunc4(){
//    A x{};
//}

//Would result in a build error saying that "A" is an unknown type, beacuse the compiler doesn't use the A from the template template parameter.
//If you wanted to be able to use the parameters, you could add additional template paramenters and use them in the function signature to get them through type deduction.
//For example, you could do this:

template<template<typename, typename> class T, typename A, typename B>
A exampleFunc5(T<A,B> var){
    A max = *var.begin();
    for(auto it = var.begin(); it != var.end(); ++it){
        if(max < *it){max = *it;}
    }
    return max;
}

//Then, if you called it like this:
//    exampleFunc5(std::vector<int> v1);
//It would deduce that T is std::vector, A is int, and B is std::allocater<int>



//Much like function parameters, template parameters (except for parameter packs) can have default arguments.
//Providing default arguments works the same way they would as with functions. If I wanted to give exampleFunc4 a default argument, I would do so like this:

//template<template<typename, typename> class T = std::vector>
//void exampleFunc4();

//After a default argument is used in the parameter list, each subsequent parameter must have a default argument as well unless they are a parameter pack or their value can be deduced from the function call.
//However, if it's not a function template then only parameters with default arguments can appear after a default argument.
//Note that in this example, ...Args is a parameter pack.

template<typename A = int, typename ...Args, typename B = int, typename C, int D = 5>
void exampleFunc6(C c){
    std::cout << "This function compiles.\n";
}


//Default arguments cannot be in an out of line definition of a member function of a class template (they need to be put in the declaration of the function), the declaration of a friend class template,
//or in a friend function template declaration (unless the declaration is a definition and there aren't any other declarations in the translation unit).
//These defaults can be merged like default funciton arguments, but a parameter can not be given default arguments twice in the same scope.

//The parameter lists within template template parameters can have default arguments too, but the arguments are only used when the template template parameter is in the scope.

template<template<typename = int> class T>
struct C{
    void func();
};

template<template<typename> class T>
void C<T>::func(){
//    T<> x = {}; <- this line produces an error if uncommented
};

//The code above defines a class template with a member function, then defines the function outside the class template.
//Though the template template parameter has a default argument in the class definition, since the function is being defined out-of-line the original template template parameter is not in scope, so its default arguments no longer apply.
//However, you can just give it a default argument again in the definition.







//--------------------------------------------------
//TEMPLATE LINKAGE AND EXPLICIT INSTANTIATION
//--------------------------------------------------

//Suppose I declared a function in a header file, like this one:
template<typename T>
void exampleFunc7(T var);
//And wanted to define it in a separate .cpp file. This would result in a build error.
//Why is this? It comes back to the idea that a function template is not actually a function. Instead it is a guide the compiler uses to create functions.
//In order for the compiler to be able to generate the code, it must be able to see the definition of the template (not just the declaration) and the types/values used to instantiate the template.
//When a C++ program is compiled, source files are compiled separately from one another and then linked together.
//Many compilers don't remember the details of one .cpp file when compiling another .cpp file.
//Therefore, when you define the template outside of the header in a separate source file, if you try to use it in your main() file
//the compiler won't see the definition of the template when compiling, so it won't be able to generate code for the instantiation
//However, if you do just define everything in the header file, you won't run into issues with the one definition rule since they're implicitly inline (to allow for multiple definitions, but not for optimization).
//The downside of this is that the template may generate the same instantiation multiple times if it is used in different .cpp files, which could cause some code bloat.


//One way to somewhat get around having to define everything in the header would be by explicitly instantiate the template.
//Previously, we've been talking about implicitly instantiating the template by using it somewhere.
//However, you can force the compiler to generate code for a given instantiation by explicitly instantiating it.
//Explicit instantiation can only be done for variable, class, and function templates.
//To explicitly instantiate a template, you declare it like normal except you don't include a template parameter list.
//Instead, you provide an argument list after the name of the template.
//In the case of function templates, you can let the compiler deduce the types.
//To explicitly instantiate exampleFunc1 you could do this:

template void exampleFunc1<int, double>(int, double);

//Or this:

template void exampleFunc1(double, double);

//If you explicitly instantiate the template within the .cpp file where you define the template, then
//the definition and instantiation will be in the same file, and you can use those instantiations of the templates in other cpp files.
//However, the problem with this is that you can only use those instantiations,
//since those are the only ones that have code generated for them.
//The better thing to do, if you use the same instantiation among multiple files, would be to use the extern keyword.
//If you put the extern keyword before an explicit instantiation, like this:

extern template void exampleFunc1(double, std::string);

//You're telling the compiler that there is another file that instantiates the template with these parameters,
//and it should just use the instantiation from that other file instead of generating new code for this cpp file.
//This effectively eliminates the potential code bloat from defining everything in the header, which essentially
//gets rid of all of the downsides of doing that.
//You should only use the extern keyword if you instantiate the template with same parameters in another file.


int main(){
    
    std::vector v1 = {1, 2 ,3};
    
    //Here, the function converts the first function argument to an int and the second to a double since the types are given explicitly.
    exampleFunc1<int, double>(5.5, 5);
    
    constexpr int y = 5;
	
	
    //Calling the function template by explicitly providing the parameters
	exampleFunc1<int>(10, 5);
	
	//Calling the function template and letting the compiler deduce the parameters
	exampleFunc1(y,0.0f);
	
	//Mixing both methods
	exampleFunc1<double>('c', 5.0);
    
    
	return 0;
}

