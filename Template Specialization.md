#Explicit Template Specialization

Suppose you have a struct template like this:

```c++
template <typename T>
struct A{
    
    using Type = T;
    
    A(T val): var(val){}
    
    /*
    There are a bunch of members here
    */
    
    template<typename T1>
    void func(){}
    
    T var;
};
```

If you instantiated the class like this: `A a_object{"Hello"};`, then T would be const char\*. This might be undesirable to you, since std::string is a lot nicer to work with than string literals, and it could cause some errors if you tried to create the const char\* on the heap. The obvious solution would be to just not let the compiler deduce the type by instantiating it like this: `A a_object<std::string>{"Hello"};`, but another solution would be to explicitly specialize the template for const char*. An explicit template specialization is a separate definition of the templated object. It essentially tells the the compiler that if the template is instantiated with a certain set of parameters, you should use the specialization's definition instead of the primary template's definition.

An explicit specialization of A would look like this:

```c++
template<>
struct A<const char*>{
        
    A(std::string val): var(val){std::cout << "Specialization used\n";}
    
    std::string var;
};
```

To say that you're specializing a template, you write the keyword template followed by <>. Then, you write the declaration of the object so that it has the same name as the original template, except that you provide the template parameters after the name, as though you were instantiating it normally (like writing `A<const char*> a1;`). Whener you instantiate a template with those parameters, the specializatino is used instead of the original template.

The definition of the specialization is not related to the original template. This means that it doesn't need to conform to the definition of the primary template, and can look completely different from it. For example, this is a valid specialization of A:

```c++
template<>
struct A<int>{
    const int Type = 5; //Type was an alias in the primary template, here it's a variable
};
```

Explicit specializations can be used with any kind of template except for alias templates. However, member templates cannot be specialized unless the enclosing class is specialized. For example, this code would produce an error because you're trying to explicitly specialize a templated member of A without specializing A as well.

```c++
template<typename T>
template<>
void A<T>::func<int>(){}
```

An explicit specialization needs to be declared after the declaration of the primary template, and can be defined within any scope that the primary template is defined. In addition, the specialization must appear before the first time you implicitly instantiate the template.
Finally, if the specialization is declared but not defined, it is like any other incomplete type (you can have references and pointers to it, but it can't be created).

An explicit specialization for a function template would look like this:

```c++
template<typename T>
void exampleFunc1(T var){std::cout <<"Regular template called\n";}

template<>
void exampleFunc1<int>(int var){std::cout << "Specialization for integers called\n";}
``` 
The specialization must have the same function signature as the original template, except with the types substitutued in. However, you can also omit the argument list if **all** the parameters can be deduced in the function, like so:

```c++
template<>
void exampleFunc1(double var){std::cout << "Specialization for doubles called\n";}
```

Explicit specializations of function templates are only inline if they are declared as such: even if the primary template is declared inline, the explicit specialization will not be inline by default. The same goes for exception specifiers like noexcept. In addition, you cannot declare or define specializations of function templates as friends. You have to declare the primary template as a friend instead. For example, this would produce a build error:

```c++
class B{
    template<>
    friend void exampleFunc1<float>(float var){};
};
```

Finally, explicit specializations of functions cannot have default arguments. This wouldn't compile:

```c++
template<>
void exampleFunc1(size_t var = 5){}
```

There are a few reasons for this to be forbidden. First of all, specializations inherit the default arguments of the base templates. Therefore, giving the specialization a default argument is technically redefining the default, which is not allowed. Furthermore, whenever you call a template function, it figures out which template to use by first looking at the primary template. If the primary template can't match the function call, the explicit specialization will not be considered. Therefore, if you leave out an argument required by the primary template it would not be able to resolve the function call properly unless the name lookup rules were changed.

&nbsp;

#Partial Template Specialization

You can also specialize a template so that it behaves differently for a set of parameters. An example of partial specialization is `std::vector<bool>`. The primary template declaration looks like this:

```c++
template<class T, class Allocator = std::allocator<T>>
class vector;
```
Whereas the partial specialization looks like this:

```c++
template<class Allocator>
class vector<bool, Allocator>;
```

The declaration of a partial specialization looks like an explicit specialization, except that the parameter list need not be empty. However, the compiler needs to be able to deduce all the parameters in the parameter list. Basically, all the parameters in the parameter list must be used in the argument list, which is the list of arguments after the template name (`bool` and `Allocater` in `std::vector<bool, Allocator>`). The declarations of partial specializations must appear within the same namespace as the primary template. If it is a partial specialization of a member template (a templated member of a class), it must be declared in the same class scope.

Unlike explicit specializations, partial specializations are still templates. Furthermore, you cannot partially specialize function templates: you just overload them instead. However, like explicit specializations partial specializations are only considered if the primary template matches.

There are a few restrictions on the argument list. First of all, it cannot be identical to the parameter list of the primary template (in other words, it must specialize something). This code would produce a build error:

```c++
template<typename T> struct A;

template<typename T1> struct A<T1>;
```

When a template with partial specializations is instantiated, the most specialized form of the template (that would still work with the instantitaiton) is used. In essence, "Template A is more specialized than template B" means that template A accepts a subset of the types that template B does. For example:

```c++
template<typename T, typename T1>
struct B;

template<typename T, typename T1>
struct B<T, T1*>; //Specialization 1

template<typename T, typename T1>
struct B<T*, T1*>; //Specialization 2
```
Specialization 2 is more specialied than specialization 1 because it only accepts pointers, whereas the accepts one parameter of any type and another of just pointers. The property is also transitive: if A is more specialized than B and B is more specialized than C, A is more specialized than C.

The exact meaning of "more specialized" is discussed in the partial ordering section because the formal rules are fairly convoluted.

Default arguments cannot appear in the argument list, for the same reasons why they can't appear in the function signature of explicit specializations of function templates. If any argument within the argument list is a pack exapansion (meaning if there is a parameter pack), it must appear at the end of the argument list. If there is a non-type parameter in the parameter list of the partial specialization, it must be used a non-deduced context (just the parameter must appear) at least once in the argument list. For example, this doesn't compile because the compiler will have to figure out what the non-type parameter is:

```c++
mplate<typename T, size_t X, size_t Y> struct C{
    C(){std::cout << "Primary template created\n";}
};

template<size_t X> struct C<int, X+1, X-1>{
    C(){std::cout << "Specialized template created\n";}
};
```
However, this specialization would not:

```c++
template<size_t X> struct C<int, X+2, X>{
    C(){std::cout << "Specialized template created\n";}
};
```
Finally, a non-type parameter in the argument list cannot be used to specialize a parameter that is a dependent type.

```c++
template<typename T1, typename T2, T2 t> struct D {};

// The type of the argument 1 depends on T, so this gives an error.
template<class T> struct D<int, T, 1>;

//The type of 1 is now expclitly given, so this is fine.   
template<class T> struct D<T, int, 1>; 

template<int X, int (*array_ptr)[X]> class E {};
int array[5];

// The type of the argument &array depends on Y, so it gives an error.
template<int Y> class D<Y, &array> {}; 
```

#Members of Specializations

If you define a member of an explicit specialization outside the class definition, you don't write the template<> unless it's a member of an explicitly specailized nested class template. For example:

```c++
template<typename T>
struct F{
    
    template<typename T1>
    struct A{}; //Primary template
    
    template<typename T1> 
    struct A<T1*>{}; //Specialization 1
    
    void func(){std::cout << "Primary function\n";}
};

template<>
struct F<int>{
    void func();
};

//You don't need template<> before this definition
void F<int>::func(){}
```
You can also specialize individual members/member templates of a class for any given implicit instantiation of the class. This lets you redefine some members without having to complete rewrite the class. However, this does use the template<> syntax. Furthermore, like explicit specializations it must appear before the instantiation of that type of template.

```c++
template<>
void F<double>::func(){std::cout << "Specialized function\n";}
```
For an `F<double>`, func will have this definition. You can also do this for members of a partial specialization.

```c++
template<typename T> //Partial specialization of F
struct F<T*>{
    void func1(){}
};

//This explicit specialization uses the definition of the partial specialization
template<>
void F<int*>::func1(){}

//Since it's using the partial specialization, this produces an error because func() doesn't exist.
template<>
void F<int*>::func(){} 
```
The members of partial specializations don't need to be defined unless they're used, since partial specializations are still templates. If a pimary template is a member of another class template, (such as `F::A`), then its partial specializations are as well. However, if you explicitly the template for an implicit specialization of the enclosing template, the partial specializations will be ignored for that instantiation.

```c++
//This definition of A is used for F<char> objects.
template<>
template<typename T>
struct F<char>::A{}; //Specialization 2

//Uses partial specialization from the primary template (specialization 1)
F<double>::A<int*> object_1;  

//Uses explicit specialization of the primary template (specialization 2)
F<char>::A<int*> object_2; 

//Uses the primary template of A
F<double>::A<int> object_3;
```