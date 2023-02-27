# Parameter Packs

If you have been using function templates a lot, you may have come across some strange behavior. More specifically, given a function template with an rvalue type parameter, like this one:

```c++
template<typename T>
void exampleFunc1(T&& val){}

static int a = 5;
```

You could call it like so: `exampleFunc1(a);`, even though val is ostensibly an rvalue reference. You're able to do this because val is actually a forwarding reference (sometimes called a universal reference). A forwarding reference is declared when a function parameter in a function template is declared as a cv-unqualified rvalue reference to a template type parameter of that function template. It's also declared when you write `auto&&`, except where auto is deduced by an initializer list. Here are a few examples to help explain when a reference is a forwarding reference, and when it's an rvalue reference.

```c++
template<typename T>
void exampleFunc2(const T&& val); 
//val is not a forwarding reference, since it's cv-qualified

template<typename T>
class A{
    template<typename T1>
    void foo(T&& x, T1&& y); 
//x isn't a forwarding reference because the type parameter is from the class, not the function.
//However, y is a forwarding reference.
    
};
```

A forwarding reference accepts expressions of any value category (lvalue, prvalue, and xvalue), much like a const lvalue reference. However, when the template parameter is deduced it also preserves the value category of the expression. If the expression is an lvalue of lvalue reference, the forwarding reference will become an lvalue reference. Otherwise, it becomes an rvalue reference (this is called reference collapsing). It also preserves the cv-qualifiers of the expression. Const expressions will remain const, volatile will remain volatile. Note that the preservation only happens when the type is deduced: if you explicitly supply it when calling the function, it won't preserve the value category.

&nbsp;

Forwarding references are used in conjunction with the `std::forward` function to enable perfect forwarding. `std::forward` is a function template defined in the utility header. It takes in a variable of any type, and just returns it. If it's a non-reference type of an rvalue reference, it returns an rvalue reference (as though you had used `std::move`). Otherwise, it returns an lvalue reference. The type of the template parameter can't be deduced, so you need to supply it explicitly. So if T is the type of the forwarding reference and val is the variable name, then you would call the function like `std::forward<T>(val)`. This is equivalent to `static_cast<T&&>(val)`. To demonstrate `std::forward`, look at the below functions:

```c++
int foo(int&){std::cout << "lvalue\n"; return 0;}
int foo(int&&){std::cout << "rvalue\n"; return 0;}

template<typename T>
void exampleFunc3(T&& val){bar(std::forward<T>(val));}
```

If you write `exampleFunc3(5);`, it will call the rvalue foo, but if you write `exampleFunc3(a);`, it will call the lvalue foo.

That's essentially all there is to forwarding: it's just there so that you can optimize move semantics. For example, in STL containers there's a member functio template called `emplace_back`, which takes in a parameter pack and uses the arguments to construct an element at the end of the container, as opposed to moving or copying it there. So if you wrote this:

```c++
std::vector<std::string> v1;
v1.emplace_back(5, 'h');
```

The `emplace_back` calls the `std::string` constructor using the arguments supplied (as in it would call `std::string(5,'h')`). Suppose forwarding references weren't a thing, and the && always referred to an rvalue reference. Then you would need to make two different versions of the function in order to make use of move semantics. Furthermore, if even one of the arguments was an lvalue, then it would have to use the lvalue version of `emplace_back`. However, with forwarding you don't have any of those problems. Look at the function below:

```c++
template<typename ...Ts>
void exampleFunc4(Ts&&... vals){
    auto a = {foo(std::forward<Ts>(vals))...};
//The auto a is just there to allow for the parameter pack expansion, you can ignore it.
}
```

If you called it like `exampleFunc4(5, a, 5, a, 5);`, then it would call the rvalue, lvalue, rvalue, lvalue, and then rvalue foo functions. Without forwarding, it would have to only call the lvalue functions.