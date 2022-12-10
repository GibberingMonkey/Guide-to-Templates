#Function Templates

As mentioned in the intro to templates section, a function template can be defined like so:

```c++
template<typename T1, class T2> 
void exampleFunc1(T1 variable1, T2 variable2){
    std::cout << variable1 << " " << typeid(T1).name() << " ";
    std::cout << variable2 << " " << typeid(T2).name() << std::endl;
}
```

Like regular functions, function templates can be overloaded. Function templates are distinct from non-templated functions. For example, you could define this non-templated function as well as the above templated function, and your code would still compile:

```c++
void exampleFunc1(int variable1, int variable2){
    std::cout << "Non-templated function called\n";
}
```

In fact, even the function call `exampleFunc1(5,3);` would still be okay even though it could technically fit both functions. In that function call, the compiler would call the non-templated function: the compiler first looks for a valid function call among non-templated functions, then function templates. If you wanted to ensure that you called the function template, you should call the function with a parameter list, like any of these three options:

```c++
exampleFunc1<int, int>(5,3);
exampleFunc1<int>(5,3);
exampleFunc1<>(5,3);
```

&nbsp;

When overloading function templates, the return type is considered to be part of the function signature. For example, your code will still copmile if you define the following two functions:

```c++
template<int I, int J>
std::array<int, I+J> exampleFunc2(std::array<int, I>, 
	std::array<int, J>)
{return std::array<int, I+J>{2};} //overload 1

template<int I, int J>
std::array<int, I-J> exampleFunc2(std::array<int, I>, 
	std::array<int, J>)
{return std::array<int, I+J>{1};} //overload 2

```

Even though they only differ in the type they return.

However, in order for the function call to be unambiguous (and therefore not produce a build error) you need to make it clear which return type you're using (such as by assigning it to a variable) or by making sure only one overload has a valid return type. These two function calls compile:

```c++
std::array<int, 4> array1 exampleFunc2<2, 2>(std::array<int, 2> {2},
	std::array<int, 2> {1});

exampleFunc2<1, 2>(std::array<int, 1> {2}, std::array<int, 2> {1});

```
The first function call uses the first overload because you're assigning it to a variable which has the type of `std::array<int, 4>` and the second overload doesn't return that. The second function call also uses the first overload. If the compiler tried to fit it to the second overload, then it would be trying to return a variable of the type `std::array<int, -1>`. Since the non-type parameter of std::array is a size_t, that would mean that the compiler would be performing a narrowing conversion, which is not allowed for non-type parameters. Thus, the second overload would not be a valid function call so the first overload is used.

&nbsp;

Two expressions involving template parameters are equivalent if they are the same, except for the names of the template parameters (which may or may not be different). For example, the function definition below is equivalent to overload 1 of exampleFunc2, and would constitute a redefinition of that overload.

```c++
template<int K, int L>
std::array<int, K+L> exampleFunc2(std::array<int, K>, 
	std::array<int, L>)
{return std::array<int, K+L>{2};}
```



Two function templates are equivalent if:

* They are in the same scope and have the same name.
* The expressions involving template parameters in the function signature are equivalent.
* They have equivalent parameter lists, meaning that for each corressponding pair of parameters:
	* The parameters are the same kind (type, non-type, etc.)
	* If they are non-type parameters, their types are equivalent.
	* If they are template template parameters, their parameter lists are equivalent.

Two expressions are functionally equivalent, but they will always evaluate to the same expression. Two function templates are functionally equivalent, except that there is at least one expression involving template parameters that is functionally equivalent:

```c++
template<int A>
void exampleFunc3(std::array<int, A+10>){}

template<int A>
void exampleFunc3(std::array<int, A+5*2>){}
```

If your code contains two function declaration sthat are functionally equivalent (but not equivalent), then it is ill-formed; no diagnostic required. This means that your code does not follow the standard, but it may still compile. Furthermore, there does not need to be any sort of warning indicating that there is an issue. In other words, it is undefined behavior and should be avoided.

#Alias and Variable Templates

These two types of templates are pretty simple. A type alias is defined like this:

```c++
using Type = int;
```

The keyword using indicates that you're using a type alias. A type alias makes the name on the left side of the = sign a synonym of the type-id (such as std::vector<int>, float, char*) on the right. In other words, you with that type alias you could write `Type a = 5;` in your program and it would create an int named a with a value of 5.

An alias template is pretty much the same exact thing, except you can now use template parameters:

```c++
template<typename T>
using vec = std::vector<T>
```

Alias templates are really only used to make aliases of types involving other templates. The only thing to watch out for is that alias templates don't allow the compiler to implicitly deduce template parameters. The folling code would produce an error:

```c++
 vec v1 = std::vector<int>{1, 2, 3, 4}; 

```

Even though the compiler should be able to deduce that the T in vec<T> would be an int. To correct the code, you'd just write 

```c++
vec v1<int> = std::vector<int>{1, 2, 3, 4};

```
instead.

&nbsp;


Variable templates are defined by placing a template declaration (the keyword template followed by a parameter list) before a variable declaration. For example:

```c++
template<typename T>
constexpr T pi = T(3.1415926535897932385L);
```
The only thing special about them is that a variable template can only be a class member if it's static. 