

#include <string>
#include <iostream>
#include <vector>


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
//PARTIAL ORDERING
//--------------------------------------------------

//When a template is instantiated, the compiler must decide which version of the template is used.
//If it's a function template, it first figures out which matching overload is most specialized
//according to the rules of partial ordering. If there is a matching explicit specialization, then that is used.
//Otherwise, it just uses the overload.

//For other templates, it first checks if any full specializations match, and if so, uses them.
//If there are matching partial specializations, it uses the rules of partial ordering to determine which
//specialization is the most specialized.

//In both cases, if the compiler has to choose between two templates that are equally specialized, the call
//is ambiguous and compilation fails.


//As mentioned previously, a simple definition to determine the most specialized template is
//If template A accepts a proper subset of the parameter lists that template B accepts, A is more specialized than B.
template<typename T> class A;
template<typename T> class A<T*>;


//In the above example, the partial specialization of E is more specialized than the primary template
//Because the primary template accepts any type, whereas the specialization only accepts pointers.
//Knowing just that is sufficient for most applications of template overloading/partial specializations,
//but if you don't know the rules really well it can lead to some really annoying errors where
//you mean to use one template but actually end up using another.


//The rules for partial ordering of class and variable templates are predicated on the rules for
//function template overloading.
//To determine which partial specialization is more specialized, you generate a fictitious function template with
//only one parameter with the class's type and specialization's template arguments.
//The more specialized function corresponds to the more specialized class template.
//Take this class template and corresponding specializations for example:
template<typename T1, typename T2> class B{};
template<typename T> class B<T, T>; //Specialization 1
template<typename T> class B<T, int>; //Specialization 2

//To determine which is more specialized, you generate the fictitious function templates
template<typename T>
void foo(B<T, T>); //This is the fictitious function template for specialization 1

template<typename T>
void foo(B<T, int>); //This is the fictitious function template for specialization 2

//The more specialized function template determines which class template specialzation is more specialized.


//Suppose you had two function templates, A and B, such that both A and B are overloads of a third template C.
//In order to determine whether A is more specialized than B, you first generate a set of unique ficticious arguments
//that you use to fill in the template parameters of A. Let's call this instantiated template A'.
//Then, you do the same thing with B, and we'll call this B'.
//We then try to see if the function signature of B' can be obtained by some instantitation of A, and if A' can be obtained from B.
//If only B' can be obtained, then B is more specialized. If only A' can be obtained, then A is more specialized.
//While doing this process, the cv qualifications of the variables matter, but the reference type does not.
//This is pretty abstract, so here are some examples with some step-by-step explanations
//There are some additional nuances covered in these examples, so you should read through them if you want to know everything.

//EXAMPLE 1:

template<typename T1, typename T2>
void exampleFunc1(T1, T2){} //Overload 1

template<typename T3, typename T4>
void exampleFunc1(T3, T4*){} //Overload 2

//Technically, the first step is to generate a unique set of fictitious template arguments (U1 and U2) that fit the parameter list of overload 1.
//However, that is overly formal. For this example, let's just choose 2 types that we don't use anywhere in the function signature.
//Let's say U1 = bool, U2 = float.
//Then we transform the template by substituting in those arguments (essentially instantiating it), leaving us with:

void exampleFunc1(bool, float); //Transformed overload 1

//Then, you do the same thing with the second overload. We'll say the types are int and std::string.

void exampleFunc1(int, std::string*); //Transformed overload 2.

//Next, we see if you can deduce the types in the parameter list of overload 2 from the signature of transformed overload 1.
//Or to put it another way, we see if there are values of T3 and T4 such that
//void exampleFunc1(T3, T4*);
//becomes
void exampleFunc1(bool, float);
//T3 = bool. But since the type of the second parameter must be a pointer, there is no T4 that would work.

//Then, we see do the same thing with overload 1 and transformed overload 2.
//We can get transformed overload 2 if T1 = int, and T2 = std::string*.
//Since we can't obtain transformed overload 1 from the original overload 2, but we can get transformed overload 2.
//Therefore, overload 2 is more specialized than overload 1.

//Note that if we had said that U2 = float* instead of float,
//then we would've been able to deduce overload 2 from transformed overload 1.
//That's where the more formal definition comes in.
//Since you're technically supposed to generate unique ficticious arguments, you need to check if you can deduce overload 2
//given any possible transformed overload 1.



//EXAMPLE 2:

template<typename T1>
void exampleFunc2(T1, T1); //Overload 1

template<typename T2>
void exampleFunc2(T2, int); //Overload 2

//To figure out which template is more specialized, let's take some arbitrary type U and substitute that into overload 1.
//This results in the function signature:
//    void exampleFunc2(U, U);

//Now, can we choose some template arguments such that overload 2 can match this function signature?
//No, since if U is not an int then the second argument can't match.

//Now, let's take some arbitrary type U1 and substitute that into overload 2.
//This results in the function signature:
//    void exampleFunc2(U1, int);

//We can't choose template arguments so that overload 1 matches this function signature, unless T1 = int.
//Therefore, neither overload is more specialized than the other, meaning a function call like
//    exampleFunc2(5, 5);
//would be ambiguous.



//EXAMPLE 3:
template<typename T, typename T1>
void exampleFunc3(T, T1){std::cout << "overload 1\n";}

template<>
void exampleFunc3<int, int*>(int, int*){std::cout << "explicit specialization of 1\n";}

template<typename T, typename T1>
void exampleFunc3(T, T1*){std::cout << "overload 2\n";}


//Suppose I called the function by writing
//    exampleFunc(5, (int*)nullptr);
//You may think that this would call the full specialization, but it instead calls overload 2.
//This goes back to the very beginning, where I mentioned the full process for how the compiler determines
//which template to use.
//For function templates, the compiler first determines the most specialized overload that matches the call,
//then it considers any explicit specializations of that overload.
//In this example, the explicit specialization is only a specialization for overload 1, which is not
//the most specialized overload that matches to the function call so it is never considered.
//If you wanted to use it, you would have to supply the template arguments explicitly, like so:
//    exampleFunc3<int, int*>(5, (int*)nullptr);

//If you wanted to explicitly specialize the second overload for the same function arguments, you could write this:
template<>
void exampleFunc3<int, int>(int, int*){std::cout << "explicit specialization of 2\n";}



//EXAMPLE 4:
template<typename T1>
void exampleFunc4(T1* val1){std::cout << "Overload 1\n";}

template<typename T1>
void exampleFunc4(const volatile T1* val1){std::cout << "Overload 2\n";}

void exampleFunc4(const volatile int* val1){std::cout << "Regular function\n";}

int a = 5;
const int* a_ptr = &a;

//Suppose you write examplefunc4(a_ptr) in your code.
//You may expect the regular function to be called, or overload 2. Instead, overload 1 is called.
//This is because when determining the best function to call, the compiler tries to avoid implicit conversions.
//If the regular function or overload 2 were used, "a_ptr" would have to be converted to const volatile integer pointer.
//On the other hand, overload 1 just deduces T1 to be a const int, requiring no extra conversions.
//Thus, the order of preference for the function call is overload 1, then the regular function, and only then overload 2
//Note that this means that if you were to try to alter val1 within overload 1 (like by writing "*val1 = 5;")
//it would result in a build error.
//A more precise way of stating this is that the compiler chooses to minimize implicit conversion sequences.
//Using the below functions, the calls exampleFunc5(5) and exampleFunc5(5.0f) both use the templated function
//since it doesn't require any conversions in the call, whereas the regular function requires a numeric conversion and
//a numeric promotion respectively.

template<typename T1>
void exampleFunc5(T1 val1){std::cout << "Template function\n";}

void exampleFunc5(double val1){std::cout << "Regular function\n";}



//EXAMPLE 5:
//References are only used to determine the order when you compare two functions which both have references
//If one does and the other doesn't, they're equally specialized. So if you had something like this:

template<typename T>
void exampleFunc6(T val){}

template<typename T>
void exampleFunc6(T& val){}

//They would be equally specialized for all lvalues you used to call them, though you could still write
//exampleFunc6(5) to call the first overload since the second can't be called with an rvalue.



//EXAMPLE 6:
//If one function has a forwarding reference and the other a lvalue reference, the one with the lvalue reference is more specialized.

template<typename T>
void exampleFunc7(T& val){std::cout << "Overload 1\n";}

template<typename T>
void exampleFunc7(T&& val){std::cout << "Overload 2\n";}

//Writing examplefunc7(a) calls overload 1, whereas something like exampleFunc7(5) would call overload 2.


template<typename T>
void exampleFunc8(const T& val){std::cout << "Overload 1\n";}

template<typename T>
void exampleFunc8(T&& val){std::cout << "Overload 2\n";}

//Just like in example 5, the compiler tries to avoid const casts when calling a function.
//So if you wrote exampleFunc8(5) or exampleFunc(a) in your code (keeping in mind that a isn't a constant variable),
//it would call overload 2 instead of overload 1.



//EXAMPLE 7:

template<typename T> struct C{
template<typename T1>
void operator* (T1){std::cout << "Member template\n";}
};

template<typename T1>
void operator* (C<char>, T1){std::cout << "Function template\n";}

//If you wrote C<char>() * 5, the compiler could use either the member operator or the global operator
//Therefore, the compiler performs some sort of ordering to determine which one to call.
//In order to facilitate this ordering, it first converts the member function into a regular function template.
//More specifically, the compiler adds another function parameter to the beginning of the list which is the type
//of the class, similar to the self parameter typically used for python member functions.
//This new parameter has the same cv-qualifiers as the object in the function call, and
//is an lvalue reference unless the member template is && ref-qualified or has no ref-qualification
//and the first parameter of the function template has && reference qualification.

//Returning to the example, if you wrote C<char>() * 5 in your code somewhere the function call would be ambiguous.
//This is because the member template is transformed into this for the purposes of partial ordering:

//template<typename T2>
//void operator* (C<char>&, T2){std::cout << "Member template\n";}

//Since the transformed template is exactly the same with regards to partial ordering, the function call is ambiguous.
//It's important to note that you don't transform the member template into this when comparing the two templates:

//template<typename T, typename T1>
//void operator* (C<T>, T1){std::cout << "Function template\n";}

//This is because as discussed in previous sections, member templates only exist for explicit specializations of a class template.
//That is, there is a member function template that exists for C<char>, and another one for C<int>, but the actual function template
//(along with other members) don't actually get defined for a general class template.



//EXAMPLE 8:
//Partial ordering only considers arguments for which there are explicit call arguments.
//Furthermore, if two templates are equally specialized for a function call and one of them has a parameter pack,
//the template with the parameter pack is less specialized for the function call

template<typename T>
void exampleFunc9(T val){std::cout << "Overload 1\n";}

template<typename T>
void exampleFunc9(T* val, int a = 5){std::cout << "Overload 2\n";}

template<typename T, typename... Ts>
void exampleFunc9(T val, Ts... vals){std::cout << "Overload 3\n";}

//If you write exampleFunc9((void*)nullptr), the compiler will ignore the second argument of overload 2 and therefore call it.
//If you write exampleFunc9(5), both overload 1 and 3 will be equally specialized but overload 3 has a parameter pack,
//so overload 1 is more specialized for the function call.


int main(){
//    exampleFunc4(a_ptr);
//    std::cout << std::boolalpha;
//    
//    exampleFunc5(5.0f);
//    
//    exampleFunc6(5);
//    
//    exampleFunc7(a);
    
    exampleFunc9((void*)nullptr, 5);
    exampleFunc9(a);
//    C<char> c_obj;
//    c_obj * 5;
//    C<char>() * 5;

}

