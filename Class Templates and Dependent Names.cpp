
#include <iostream>
#include <vector>


//NEED MEMBER TEMPLATES


//--------------------------------------------------
//CLASS TEMPLATES
//--------------------------------------------------


//Class templates are declared as normal structs, classes, and unions, except that there is a template delcaration in front of it:

template<typename T>
struct A{
    
    using Type = T;
    
    A(): var(T()) {} //Default constructor
    
    A(T): var(T()) {} //Constructor which allows type deduction for template parameters.
    
    A(const A& other): var(other.var) {} //Copy constructor, also allows for type deduction
    
    A operator= (const A& other){} //Copy assignment operator
    
    template<typename T1>
    A operator= (A<T1>&& other){} //Templated move assignment operator
    
    void func1(T var){std::cout << "func1 called\n";}
    
    void func2(){std::cout << var.size() << "\n";}
    
    T var;
    
    static T static_var;
    
};

//The template parameter(s) can then be used in the entire class.

//If something can be defined within a regular class, then it can also be defined within a class template.

//When you create an instance of a class template, you typically supply the template parameters explicitly, like by writing:

std::vector<int> v1;

//However, as of C++ 17 you can let the compiler deduce the template parameters.
//In order to do so, the constructor you're using must have all the template parameters in the function signature.
//In the above class template, the default constructor doesn't have a way to deduce the type parameter,
//But the other two constructors do.

A a_object_1 = A(5); //Deduces T to be an int
A a_object_2 = a_object_1; //The compiler deduces T in a_object_2 to be the same as the T in a_object_1


//It's important to note that the template parameters are substituted into the members of the class when it is first instantiated.
//As such, none of the members are truly templates (unless you put a template declaration before them), and you cannot use
//Parameter lists when calling functions, accessing variables, etc.
//Basically, trying to do this will result in an error saying that func1 isn't a template but you're giving it template arguments:

//    a_object_1.func1<int>(5);

//This function call also produces a build error, since the type of T is already deduced and you can't convert a const char* to an int:

//    a_object_2.func1("Hello");

//Therefore, when A is used in the class template, the generated code will essentially be A<T>.
//In fact, you can name your constructor A<T>, though only within the class definition itself (you can't refer to it as A<T> in an out-of-line definition).
//In other words, you wouldn't be able to call the copy constructor for a_object_2 using an instance of A<double> unless you made it into a templated operator overload, like with the move assignment operator.

//However, while the members aren't templates, they still aren't actually compiled when you instantiate the class unless they are used
//Although ints don't have a size() member function, I can still create an instance of the A struct template with an int,
//as long as I don't use func2 for that instance because func2 won't be generated for the class.

//Furthermore, a class template isn't actually instantiated if you just have a pointer to that type.
A<double>* ptr; //This doesn't generate any code for A<double>

//It's only when you access members using that pointer that the compiler generates the code.
//The following line would instantiate the class and generate code for func1:

//ptr->func1(5.0);



//Out of line definitions of members are pretty similar to how it works with regular classes, except that it needs a template declaration to give the parameters to the class name in the access specifier:

template<typename t> //The parameter list doesn't need to have the same names as in the class template.
t A<t>::static_var = t(); //If the <t> wasn't there, you would get a build error beacuse it wouldn't know what type A was.


//Static members are only shared between instances of the same type: A<int>::static_var and A<double>::static_var are not the same.


//Nested classes (A class defined wthin another class) can be templates as well, and can be defined within class templates.
//However, nested classes cannot have member templates.


//--------------------------------------------------
//INHERITANCE AND DEPENDENT NAMES
//--------------------------------------------------


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

//A class can inherit from a specific version of template, as is done with the code above.
//Similarly, a class template can inherit from a non-templated class.
//This works exactly how inheritance from non-templated classes works.





//Inheriting from a class template itself is more complicated.
//For starters, the derived class needs needs to be a template as well. Moreover, it needs to at least have the template parameters the base class has.
//This is because whenever you create an instance of the derived class, you need first construct the base class.
//Therefore, you need to supply the template parameters of the base class in order to construct it.


using Type = double;

double var = 3.1415;

template<typename T>
struct D: public A<T>{
    
    void DFunc1(){
        var1 = 5.8;
        std::cout << var << std::endl;
    }
    
    void DFunc2(){func1(T());}
    
    Type var1;
    
};

D<int> d_object_1;

//However, that isn't all there is to it. If you run this code in main():

//    d_object_1.DFunc1();

//You would expect that var1 be changed to 5 (converting 5.8 to an int), and that 0 will be printed to the console.
//Instead, var1 becomes 5.8 and 3.1415 is printed to the console. In addition, if you try to call DFunc2 it will produce a build error.
//As an aside, this class template may work as expected with some compilers. Such behavior is incorrect.
//For some reason, D is using the global definitions of Type and var, though it should be inheriting those from A.

//And D is inheriting the base class members; you could write this in the main() function and it would still compile:

//    d_object_1.func1(5);

//You can also access the var member variable and it will still be an int:

//    c_object_1.var = 5; //This still compiles in main()

//So what's going on? This strange behavior is due to dependent names.
//A dependent name is a name that depends on a template parameter in some way.
//Non-dependent names don't depend on template parameters.
//In D, Type and var don't depend on a template parameter, so they are non-dependent names.
//However, the base class template A<T> does depend on a template parameter, so it is a dependent type.
//Therefore, all the the members of A<T> are dependent types as well, since they depend on A<T>.
//When the compiler performs name lookup (that is, when it decides what the name of a type, function, etc. refers to), it does not look at dependent names.
//So, in order to use the base class members, you need to make them dependent names.
//There are a few ways you can do this.


//First of all, the this pointer is implicitly dependent within templates. Thus, you can just write
//this->name
//for every base member.
//You could also specify that you're looking for the base class member by doing something like A<T>::var
//Finally you could write
//    using A<T>::func1
//Before calling func1, or just within the class body.

//Types defined within the base class work a bit differently.
//You may think that you could write A<T>::Type var1; within D, or even using A<T>::Type;, but this would be wrong.
//This is because A<T>::Type is not necessarily a type.
//This has to do with template specialization, which is covered in the next section.
//But basically, you can define A for a specific parameter, and in this definition you can change Type to be whatever you want,
//Or even not include it entirely.
//Therefore, you need to tell the compiler that you're referring to a type, which you do by writing the typename keyword in front of it.
//A derived class defined correctly should look something like this:


template<typename T>
struct E: public A<T>{
    
    using A<T>::func1;
    
    void EFunc1(){
        var1 = 5.8;
        std::cout << this->var << std::endl;
    }
    
    void EFunc2(){func1(T());}
    
    typename A<T>::Type var1;
    
};






int main(){

    
    
    a_object_1.static_var = 0;
    
    A<double> a_object_3;
    
    a_object_3.static_var = 1.2;
    
    std::cout << A<double>::static_var << ' ' << A<int>::static_var << std::endl;
    
        
    
    return 0;
}
