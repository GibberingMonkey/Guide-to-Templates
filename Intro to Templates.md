# Intro to Templates
Templates are how c++ implements generic programming. There are four kinds of templates: template functions, template classes, template variables, and template aliases. Of these four, template functions and classes are the most common and most useful. It should be noted that templates aren't actually classes, functions, etc. Instead, they are a guide for the compiler on how to generate those things based on some parameters that are provided when the template is instantiated. Since templates are generating classes and functions, their instantiation occurs during compile time. Thus, they do not add any runtime cost. 

However, templates are compiled differently from other code: they employ two-phase lookup. In the first phase, the template is checked for syntax. In the second phase, the arguments supplied to the template are substituted in and it's check again (though some compilers skip the first phase entirely). Notably, this second phase only occurs when you instantiate the template (since that is when the arguments are given). If you never use the template the second phase will never happen. This can lead to some errors that don't always appear when you compile your code. For example:

```c++
template<typename>
struct A{
    A(int){}
};

template<typename T>
struct B: public A<T>
{
    B(){}
};

```

The above code defines two class templates, A and B, such that B is derived from A. The default constructor for A is implicitly deleted, so B must explicitly construct A. It would be like trying to do this:

```c++
struct A{
    A(int){}
};

struct B: public A
{
    B(){}
};
```
The non-templated code results in a build error, but the templated code does not unless you actually try to instantiate B.

### Template Parameters

A function template can be defined like this:

```c++
template<typename T1, class T2>
void exampleFunc1(T1 variable1, T2 variable2){
	std::cout << variable1 << " " << typeid(T1).name() << " ";
	std::cout << variable2 << " " << typeid(T2).name() << std::endl;
}
```
The template keyword tells the compiler that you're declaring/defining a template, which is then followed by a parameter list enclosed by < and >. A parameter list is a comma separated list of four types of parameters: type parameters, non-type parameters, template parameters, and parameter packs. Each of these parameters must be known at compile time when the template is instantiated. Parameter packs are a complicated subject on their own, and will be discussed later in their own section.

The above function template has two type parameter. A type parameter is declared within a parameter list by writing a type parameter key (which is either the keyword "typename" or "class", there is no difference in this context), followed by a name. Though the name is optional, I cannot think of a situation where you would want a type parameter without a name. When a template has a type parameter, it means that you need to supply a type to the template when you call it, either implicitly or explicitly. The name of the parameter is then used as an alias of the type (if the above function was instantiated with T1 being an int, it would be equivalent to `using T1 = int; `). The type can be other templated objects, like std::vector, but they need to be instantiated (you can't just have the type be std::vector, it should be something like std::vector<int>). You should use type parameters when you want a function to mostly do the same thing but take in bunch of different types, like a sort of print function.

There are two ways to call a template with a type parameter. You could provide the types enclosed by <> after the name of the template, like this:

```c++
exampleFunc1<int, double>(5, 7.7);
```

This is how you typically instantiate an instance of std::vector, and is called explicit instantiation.

Alternatively, you could instantiate it like you would any regular function and let the compiler deduce the types. If you call the function like this:

```c++
exampleFunc1(3.0f, "Hello");
```
The compiler tries to figure out the types based on the function call. In this case, it deduces T1 to be a float and T2 to be a char*. The nuances of type deduction will be covered in a separate section.

You can also combine the two methods: ` exampleFunc1<int>(5, 7);` is a valid function call. However, the explicit type declarations will always be in the order of the parameter list in the template declaration.

&nbsp;


```c++
template<int N>
void exampleFunc2(){
	std::cout << N << std::endl;
}
```

The above template has a non-type parameter. These parameters have the type specified, but the actual value of the variable needs to be supplied. However, the type can also depend on another template parameter, like so:

```c++
template<typename T, T N>
void exampleFunc3(){
    std::cout << N << std::endl;
}
```

Basically, the syntax is the same as function parameters. However, there are several key differences. For starters, the non-type parameter cannot be of any type, since the value of the parameter needs to be known at compile time. They can only be integral types (int, char, etc.), enums, non-void pointers and references, and std::nullptr_t (the type of nullptr). You can also have function types and arrays, but those are automatically replaced by pointers. Notably, this means that you can't use floats or doubles as a non-type parameter, even though these are easily evaluated at compile time. There isn't a satisfying explanation for this, and that's probably why you can have floating point numbers as non-type parameters in C++ 20. In order to use pointers or references, the address of the thing you're referencing must be known at compile time. For this reason, any object you pass into a pointer or reference non-type parameter must be in static memory.

You may wonder what the point of non-type parameters are, since you could just as easily write a function like the one below that would do the same thing as `exampleFunc2`:

```c++
void normalFunc(int N){
	std::cout << N << std::endl;
}
```
The main difference is that when you specify a non-type parameter, it must be known at compile time. This lets you do things that you wouldn't normally be able to do in functions. For example, in exampleFunc2 you could create an array of size N, which you can't do with exampleFunc3 (unless your compiler supports variable length arrays). Furthermore, non-type parameters have stricter rules for implicit conversions. The only type of conversions allowed when supplying non-type parameters are converted constant conversions, which are these conversions:

* Users defined conversions which are constexpr
* Lvalue to rvalue and qualification (like inst to const int) conversions
* Integral promotions (going from a smaller integral type to a larger one, like int to long int)
* Array to pointer, function to pointer, and converting pointers to and from nullptr
* Functino pointer conversions (pointer to a noexcept function to a pointer to a regular function)
* Non-narrowing integral conversions (you can't lose preceision in a conversion)
* And if any reference binding takes place, it doesn't create a temporary object.

Basically, if the conversion can happen at compile time and (if the conversion is not user-defined) doesn't lead to a loss in data, you can use the implicit conversion.

&nbsp;

```c+++
template<template<typename, typename> class T>
void exampleFunc4();
```

The above function declaration has a template template parameter. This parameter can either be a class or alias template, but not a function or variable template. It's declared as you could normally declare a type parameter, except that it has another template declaration before it. The inner template declaration needs to be able to have the same axact template parameters (in number and type) as the template you supply to it during instantiation. For example, to use std::vector you need two type parametrs, even though you'd normally instantiate it without providing an allocater. This parameter is the actual template (std::vector, not std::vector<int>), so whenever you use it in the templated object you need to supply its arguments.
While the parameters of the template template parameter can be given names, you can't actually use those names within the function template. For example, this function:

```c++
template<template<typename A> class T>
void exampleFunc4(){
    A x{};
}
```
Would result in a build error saying that "A" is an unknown type, since the compiler doesn't use the A from the template template parameter. If you wanted to be able to use the parameters, you could add additional template parameters and usse them in the function signature to get them through type deduction. For example, you could do this:

```c++
template<template<typename, typename> class T, typename A, typename B>
A exampleFunc5(T<A,B> var){
    A max = *var.begin();
    for(auto it = var.begin(); it != var.end(); ++it){
        if(max < *it){max = *it;}
    }
    return var;
}
```
Then, if you called it like this: `exampleFunc5(std::vector<int> v1);` it would deduce that T is std::vector, A is int, and B is std::allocater<int>

&nbsp;

Much like function parameters, template parameters (except for parameter packs) can have default arguments. Providing default arguments works the same way they would as with functions. If I wanted to give exampleFunc4 a default argument, I would do so like this:

```c++
template<template<typename, typename> class T = std::vector>
void exampleFunc4();
```
After a default argument is used in the parameter list, each subsequent parameter must have a default argument as well unless they are a parameter pack or their value can be deduced from the function call. However, if it's not a function template then only parameters with default arguments can appear after a default argument. Note that in the example below, ...Args is a parameter pack.

```c++
template<typename A = int, typename ...Args, typename B = int, typename C, int D = 5>
void exampleFunc6(C c){
    std::cout << "This function compiles.\n";
}
```
Default arguments cannot be used in an out-of-line definition of a member function of a class template (they need to be put in the declaration of the function), the declaration of a friend class template, or in a friend function template declaration (unless the declaration is a definition and there aren't any other declarations in the translation unit). These defaults can be merged like default function arguments, but a parameter cannot be given default arguments twice in the same scope. The parameter lists within template template parametrs can have default arguments too, but the arguments are only used when teh template template parameter is in the scope.

```c++
template<template<typename = int> class T>
struct C{
    void func();
};

template<template<typename> class T>
void C<T>::func(){
    T<> x = {};
};
```
The code above defines a class template with a member function, then defines the function outside the class template. Though the template template parameter has a default argument in the class definition, since the function being defined out-of-line the original template template parameter is not in scope. Thus, its default arguments no longer apply. However, you can just give it a default argument again in the definition.

### Template Linkage

Suppose I declared a function in a header file, like this one:

```c++
template<typename T>
void exampleFunc7();
```
And wanted to define it in a separate .cpp file. This would result in a build error. Why does this happen? It comes back to the idea that a function template is not actually a function, but a guide the compiler uses to create functions. In order for the compiler to generate the function when you instantiate it, it must be able to see the definition of the template (not just the declaration) and the types/values used to instantiate the template. Because of the separate compilation model, many compilers don't remember the details of one .cpp file when compiling another .cpp file. Therefore, when you define the template outside of the header, the compiler won't be able to see the definition in main() so it won't be able to instantiate the template.  This is a bit of an oversimplification, but it serves well enough for the purposes of this guide. However, if you do just define everything in the header file, you won't run into issues with the one definition rule since it does not apply to class and function templates. The downside of this is that templates may generate the same instantiation multiple times if it is used in .cpp files, leading to some code bloat.

One way to somewhat get around needing define everything in the header is to explicitly instantiate the template. Previously, we've been talking about implicitly instantiating the template by using it somewhere. However, you can also force the compiler to generate code for a given instantion by explicitly instantiating it. Explicit instantiation can only be done for variable, class, and function templates. To explicitly instantiate a template, you declare it like it like normal except that you don't include a template parameter list. Instead, you provide an argument list after the name of the template. In the case of function templates, you can let the compiler deduce the types. In addition, you need to substitute arguments in for any template arguments in the function signature. To explicitly instantiate exampleFunc1, you could write this:

```c++
template void exampleFunc1<int, double>(int, double);
```
Or this:

```c++
template void exampleFunc1(double, double);
```
If you explicitly instantite the template within the .cpp file where you define the template, then the definition and instantiation will be in the same file, and you can use that instantiation of the template in other cpp files. However, the problem with this is that you can only use those instantiations, since those are the ones with code generated for them. The better thing to do, if you use the instantiation among multiple files, would be to use the extern keyword. If you put the kextern keyword before an explicit instantiation, like this:

```c++
extern template void exampleFunc1(double, std::string);
```

You're telling the compiler that there is another file that instantiates the template with these parameters, and it should use the instantition from that other file instead of generating new code for this cpp file. This effectivly eliminates the potentional code bloat from defining everything in the header, which essentially gets rid of all the downsides of doing that. You should only use the extern keyword if you instantiate the template with same parameters in another file.
