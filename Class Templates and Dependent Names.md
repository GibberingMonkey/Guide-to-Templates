# Class Templates

Class templates are declared as normal structs, classes, and unions, except that there is a template delcaration in front of it:

```c++
template<typename T>
struct A{
    
    using Type = T;
    
    A(): var(T()) {} 
        
    //Constructor which allows type deduction for template parameters.
    A(T): var(T()) {} 
        
    //Copy constructor, also allows for type deduction
    A(const A& other): var(other.var) {} 
    
    A operator= (const A& other){}
    
    //Templated move assignment operator
    template<typename T1>
    A operator= (A<T1>&& other){} 
    
    void func1(T var){
    std::cout << "func1 called\n";
    }
    
    void func2(){std::cout << var.size() << "\n";}
    
    T var;
    
    static T static_var;
    
};

```

The template parameters can then be used in the entire class. Class templates don't have any special restrictions on the types of members they hold: if something can be declared within a non-templated class, it can be declared within a class template.

When you instantiate a class template, you typically supply the template parameters explicitly, like so: 

```c++
std::vector<int> v1
```

However, as of C++17 you can let the compiler deduce the template parameters. In order to do so, the constructor you're calling must use all the template parameters in the function signature. In the above template, the default constructor doesn't have a way to deduce the type parameter, but the other two constructors do. So when you construct to A objects like so:

```c++
A a_object_1 = A(5);
A a_object_2 = a_object_1;
```
The compiler deduces T to be an int for the first call, and deduces T in a\_object\_2 to be the same as the T in a\_object\_1.

&nbsp;

It's important to note that the template parameters are substituted into the members of the class when it is first instantiated. As such, none of the members are truly templates (unless you put a template declaration before them, like with the move assigment operator). Therefore, you cannot use parameter lists when calling functions, accessing variables, etc. Basically, this line of code will result in an error saying that func2 is not a template but you're giving it template arguments: `a_object_1.func1<int>(5);`

This function call also results in a build error, since the type of T is already deduced to be an int: `a_object_2.func1("Hello");`

Since this substitution needs to happen at the point of instantiation, whenever you refer to A (without giving it parameters) within the template, it actually just means A<T>. In fact, you can even name your constructor A<T>, though you can only do this within the class definition itself (you can't refer to the constructor as A<T> in an out-of-line definition).

However, while the members of class templates aren't really templates, they still aren't compiled (that is, they aren't used to generate code) unless you actually use the member. For example, although ints don't have a size() member function, I can still create an instance of the A class template with an int, as long as I don't use func2 for that object.

Furthermore, a class template isn't actually instantied if you just have a pointer to that type. For example, the line `A<double>* ptr;` doesn't generate any code for A<double>. It's only when you access members through the pointer that the compiler generates the code. The following line would specialize the template and generate code for func1: `ptr->func1(5.0);`

Out-of-line definitions of members are pretty similar to how it works with regular classes, except that it needs a template parameter list that's the same as the class template's parameter list. This means that the parameters are the same types and in the same order, but not the same names. You then give these parameters to the class name in the access specifier. For example, to define the static variable of A you could write:

```c++
template<typename t>
t A<t>::static_var = t{};
```

Static members are only shared between specializations of the same type: A<int>::static\_var and A<double>::static\_var are not the same variable.

Nested classes (A class defined within another class) can be templates as well. However, they cannot have member templates.



&nbsp;

# Inheritance and Dependent Names


```c++
struct B: public A<int>{
    using A<int>::A;
    
    B(int _var): A<int>(_var){}
    
    void func(){std::cout << var << std::endl;}
};


template<typename T>
struct C : B{
    using B::B;
    
    C(int _var){var = _var;}
    
    void func3(){std::cout << var << std::endl;}
};

```


A class can inherit from a specialization of a template, and a class template can inherit from a non-templated class, as is done with the code above. This works exactly how inheritance with non-templated classes works.

&nbsp;

Inheriting from a class template is more complicated. For starters, the derived class as well. Moreover, it needs to at least have the template parameters the base class has (if the base class has two type parameters, the derived class needs to have at least one type parameter). This is because when you try to instantiate a derived class, you need to first construct the base class. Therefore, you need to supply the template parameters to the base class, which means that you need to supply the parameters to the derived class when you instantiate it. The class below inherits from the A class template defined above:

```c++
using Type = double;

double var = 3.1415;

//ignore the above lines for now
template<typename T>
struct D: public A<T>{
    
    void DFunc1(){
        var1 = 5.8;
        std::cout << var << std::endl;
    }
    
    void DFunc2(){func1(T());}
    
    Type var1;
    
};
```

Now if we were write this in main():

```c++
D<int> d_object_1;
d_object_1.DFunc1();
```

We would expect that the var1 member variable be changed to 5 (converting 5.8 to an int), and that 0 will be printed to the console. Instead, var1 becomes 5.8 and 3.1415 is printed to the console. In addition, if you try to call DFunc2 it will produce a build error. [^1] 
[^1]: This class template may work as expected with some compilers. Those compilers are not compliant with the standard.

D seems to be using the global definitions of Type and var, though it should be inheriting those from A. And D *is* inheriting the base class members; you could write this in main() and it would still compile: `d_object_1.func1(5);`

You can also access the var member variable and it will still be an int: `    c_object_1.var = 5; //This still compiles in main()`

So what's going on? This stranger behavior is due to dependent names. A dependent name is a name that depends on a template parameter in some way. Non-dependent names don't depend on template parameters. In D, Type and var don't depend on a template parameter, so they are non-dependent names. However, the base class template A<T> does depend on a template parameter, so it is a dependent type. Therefore, all the members of A<T> are dependent types as well. When the compiler performs name lookup (when it decides what the name of a type, function, etc. refers to), it does not look at dependent names. So, in order to use the base class members, we need to tell the compiler to look for dependent names. There a few ways to do this.

One way is with the this pointer. The this pointer is implicitly dependent within templates. Thus, you could just write `this->name` for every base member. Alternatively, you could also specify that you're looking for the base class member by using the scope operator, writing something like `A<T>::var`. Finally, you could write `using A<T>::func1` before calling func1, or just within the derived class's body.

Types defined within the base class work a bit differently. You may think you could just write `A<T>::Type var1;` or `using A<T>::Type;` within D, but this still wouldn't work. This is because `A<T>::Type` is not necessarily a type. This has to do with template specialization which is covered in the next section. But basically, you can define A to behave differently for a specific parameter (like A<double>), and in that definition you can change Type to be whatever you want, or even not include it entirely. Therefore, you need to tell the copmiler that you're referring to a type, which is done by writing the typename keyword in front of it. A derived class defined correctly should look like this:

```c++
template<typename T>
struct E: public A<T>{
    
    using A<T>::var;
    
    void EFunc1(){
        var1 = 5.8;
        std::cout << var << std::endl;
    }
    
    void EFunc2(){this->func1(T());}
    
    typename A<T>::Type var1;
    
};
```
