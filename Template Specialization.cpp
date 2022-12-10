#include <iostream>
#include <string>
#include <vector>

//--------------------------------------------------
//EXPLICIT TEMPLATE SPECIALIZATION
//--------------------------------------------------


//Suppose you have a struct like this:
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

//If you instantiated the class like this:
//A a_object{"Hello"};  //<- this normally line compiles, but will cause a build error because of subsequent code if left uncommented.

//Then var would be a const char*.
//This might be undesirable to you: string literals are typically not the easiest thing to work with, at least compared to std::string.
//And if you were trying to create something on the heap of type T, then it would probably mess up your class.
//Luckily, you can get around this by using template specializations.
//A template specialization is a separate definition of the templated object which basically tells the compiler that if the object
//is instantiated with a certain set of parameters, the compiler should use the specialization instead of the normal template.

//An explicit specialization of A would look like this:
template<>
struct A<const char*>{
        
    A(std::string val): var(val){std::cout << "Specialization used\n";}
    
    std::string var;
};

//To say that you're specializing a template, you write the keyword template followed by <>.
//Then, you write the declaration of the object so that it has the same name as the original template,
//except you provide the parameters after the name, as though you were instantiating it normally (like writing A<int> a1; in main()).
//Whenever you instantiate a template with those parameters the specialization is used instead of the original template.
//The definition of the specialization does not need to look anything like the original template.
//For example, this is a valid specialization of A:

template<>
struct A<int>{
    const int Type = 5;
};

//Incidentally, this specialization is a good example for why the standard says you should use typename when referring
//to an inherited dependent type alias (discussed in the previous section).
//In the base template A, Type is a type.
//In this specialization, Type is a variable.

//Explicit specializations can be used with any kind of template except for alias templates.
//However, member templates cannot be specialized unless the enclosing class is specialized.

//template<typename T>
//template<>
//void A<T>::func<int>(){} // <- this produces an error because A is unspecialized.


//The specialization needs to be declared after the declaration of the primary template
//and can be defined within any scope that the primary template is defined.
//In addition, the specialization must appear before the first time you implicitly instantiate the the template.
//If you uncommented line 28, it would cause a build error because you implicitly instantiated a template for const char* before you specialized it.
//Finally, if the specialization is declared but not defined, it is like any other incomplete type (you can have references and pointers to it, but it can't be created).


//An explicit specialization for a function template would look like this:

template<typename T>
void exampleFunc1(T var){std::cout <<"Regular template called\n";}

template<>
void exampleFunc1<int>(int var){std::cout << "Specialization for integers called\n";}

//The specialization must have the same function signature as the original template, except with the types substituted in.
//However, you can also omit the template parameter list if ALL the parameters can be deduced in the function, like so:

template<>
void exampleFunc1(double var){std::cout << "Specialization for doubles called\n";}

//Explicit specializations of funtion templates are only inline if they are declared as such:
//Even if the base template is declared inline, the explicit specialization will not be inline by default.
//The same goes for exception specifiers like noexcept.
//In addition, you cannot declare or define specializations of function templates as friends.
//This would produce a build error:

//class B{
//    template<>
//    friend void exampleFunc1<float>(float var){};
//};

//Finally, explicit specializations of functions cannot have default arguments.
//This doesn't compile:

//template<>
//void exampleFunc1(size_t var = 5){std::cout <<"Regular template called\n";}


//There are a few reasons for this to be forbidden.
//First of all, specializations inherit the default arguments of the base templates.
//Therefore, giving the specialization a default argument is technically redefining it, which is not allowed.
//Furthermore, whenever you call a templated function, it figures out which template to use by looking at the base template.
//Therefore, if you leave out an argument required by the primary template it wouldn't be able to resolve the function call
//unless the name lookup rules were changed.


//--------------------------------------------------
//PARTIAL TEMPLATE SPECIALIZATION
//--------------------------------------------------

//You can also choose to partially specialize a template.
//An example of this is std::vector<bool>. The primary template declaration looks like this:
template<class T, class Allocator = std::allocator<T>>
class vector;

//Whereas the partial specialization for std::vector<bool> looks like this:
template<class Allocator>
class vector<bool, Allocator>;


//The declaration of a partial specialization looks like an explicit specialization, except that the parameter list need not be empty.
//However, the compiler needs to be able to deduce all the parameters in the parameter list.
//Basically, it all the things in the parameter list must be used in the argument list, which is defined as
//the arguments after the template name ("bool" and "Allocator" in std::vector<bool, Allocator>).
//The declaration of partial specializations must appear within the same namespace as the primary template.
//If it is a partial specialization of a member template (a templated member of a class), it must be declared in the same class scope.
//Unlike explicit specializations, partial specializations are still templates.
//Furthermore, you cannot partially specialize function templates. You have to overload them instead.
//The partial specializations aren't found in name lookup: they are only considered if the primary template is found.
//As such, if a using declaration refers to a primary template, the partial specializations will still be used with it.

template<typename T>
using vec = std::vector<T>;

vec<bool> example; //This still uses the partial specialization of std::vector

//There are a few restrictions on the argument list. First of all, it cannot be identical to the parameter list of the primary template
template<typename T> struct A;

//template<typename T1> struct A<T1>; //The argument list is identical to the parameter list of the primary template, so it's not a partial specialization

//In addition, it must be more specialized than the primary template.
//In essence, "Template A is more specialized than template B" means that template A accepts a subset of the types than template B does,
//such as by only accepting pointers instead of all types.

template<typename T, typename T1>
struct B;

template<typename T, typename T1>
struct B<T, T1*>; //Specialization 1

template<typename T, typename T1>
struct B<T*, T1*>; //Specialization 2

//Specialization 2 is more specialized than specialization 1 because it only accpets pointers, whereas the other accepts
//one parameter of any type and another of just pointers.


//The exact meaning of "more specialized" is discussed in the partial ordering section, since the formal rules are very involved.


//Default arguments cannot appear in the argument list, for the same reasons why they can't appear in full specializations.
//If any argument is a pack expansion (meaning if there's a parameter pack) it must be at the end of the argument list.
//If there is a non-type parameter in the parameter list of the partial specialization (as opposed to the argument list),
//it must be in a non-deduced context (just the parameter must appear) at least once in the argument list. For example:

template<typename T, size_t X, size_t Y> struct C{
    C(){std::cout << "Primary template created\n";}
};

//template<size_t X> struct C<int, X+1, X-1>{
//    C(){std::cout << "Specialized template created\n";}
//};

//If uncommented, this creates a build error because the compiler will have to figure out what X is.

template<size_t X> struct C<int, X+2, X>{ // <- This does not, because X is explicitly given to the template.
    C(){std::cout << "Specialized template created\n";}
};


//Finally, a non-type parameter in the argument list cannot be used to specialize a parameter that is a dependent type.

template<typename T1, typename T2, T2 t> struct D {};
//template<class T> struct D<int, T, 1>;   // The type of the argument 1 depends on T, so this gives an error.
template<class T> struct D<T, int, 1>; //The type of 1 is now expclitly given, so this is fine.


template<int X, int (*array_ptr)[X]> class E {};
int array[5];
//template<int Y> class D<Y, &array> {}; // The type of the argument &array depends on Y, so it gives an error.
template<int (*array_ptr)[5]> class E<5, array_ptr> {}; //This is fine.



//--------------------------------------------------
//MEMBERS OF SPECIALIZATIONS
//--------------------------------------------------

//If you define a member of an explicit specialization outside the class definition, you don't write template<>,
//unless it's a member of an explicitly specialized nested class template.

template<typename T>
struct F{
    
    template<typename T1>
    struct A{}; //Primary template
    
    template<typename T1> //Partial specializations of member templates must appear within the definition.
    struct A<T1*>{}; //Specialization 1
    
    void func(){std::cout << "Primary function\n";}
};

template<>
struct F<int>{
    void func();
};

void F<int>::func(){} //You don't need template<> before this definition


//You can redefine individual members/member templates of a class for any given implicit instantiation of the class.
//This lets you redefine some members without having to completely rewrite the class.
//However, this does use the template<> syntax.
//Furthermore, like explicit specializations it must appear before the instantiation of that type of template

template<>
void F<double>::func(){std::cout << "Specialized function\n";} //For an F<double>, func will have this definition.

//You can also do this for members of a partial specialization.

template<typename T> //Partial specialization of F
struct F<T*>{
    void func1(){}
};


template<>
void F<int*>::func1(){} //This explicit specialization uses the definition of the partial specialization

//template<>
//void F<int*>::func(){} //Since it's using the partial specialization, this produces an error because func() doesn't exist


//The members of partial specializations don't need to be defined unless they're used, since partial specializations are still templates.

//If a primary template is a member of another class template (such as F::A), then its partial specializations are as well.
//However, if you explicitly specialize the template for an implicit specialization of the enclosing template,
//the partial specializations will be ignored for that instantiation.

template<>
template<typename T>
struct F<char>::A{}; //This definition of A (specialization 2) is used for F<char> objects.

F<double>::A<int*> object_1;  //Uses the partial specialization from the primary template (specialization 1)
F<char>::A<int*> object_2; //Uses full specialization of the primary template (specialization 2)
F<double>::A<int> object_3; //Uses the primary template of A


int main(){
    return 0;
}
