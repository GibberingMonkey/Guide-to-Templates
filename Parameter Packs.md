# Parameter Packs


```c++
template<typename... Args, template<typename> class ...Args1, typename... Args2>
void exampleFunc1();

template<typename... Args>
void exampleFunc2(Args ...args);
```

The above functions contain template parameter packs, the second also contains a function parameter pack. A template parameter pack (a parameter pack within a template parameter list) is a parameter that can accept any number of parameters, including 0. It can accept any type of parameter (non-type, type, or template), but it can only accept one type of parameter. To declare a parameter pack, you write the template parameter as normal but put an ellipsis prior to where the name of the parameter would normally go (though parameter packs don't need a name). A template which has a parameter pack is called a variadic template.

A function parameter pack (a parameter pack within a function parameter list) is a parameter that accepts any number of function arguments, including 0. To declare a function parameter pack, you use a template parameter pack as the type and put an ellipsis before the name (which is optional). The function parameter pack then takes in the same number of arguments as the template parameter pack, and the type of each element in the function parameter pack is the corresponding element in the template parameter pack. For example, this is not a valid function call `exampleFunc2<int, int, int>(1,1);` because the size of the parameter packs don't match, but these function calls are all valid:

```c++
exampleFunc2<int, int, int>(1, 1, 2);

//Here, the compiler deduces that the template parameter pack is int, int, int.
exampleFunc2(1, 1, 2);
exampleFunc2(1, 1);
exampleFunc2<int, int>(1, 1);
```

In function templates, a template parameter that appears after a template parameter pack must have a default argument, be deducible from the function signature, or be another parameter pack. If there are two parameter packs in a row, they must be of different parameter types (type, template, or non-type). In otehr templates, there can only be one parameter pack and it must be the last parameter. If you want to find out the number of elements in a parameter pack, you can use the `sizeof...` operator. Given a parameter pack called Args with N elements, `sizeof...(Args)` will return N. This operator is constexpr, so the value it returns is evaluated at compile time.


In order to access the elements in a given parameter pack, you need to use a pack expansion. A pack expansion occurs when a pattern is applied to the name of a parameter pack and is then followed by an ellipsis, it is expanded into a number of instantiations of the pattern where the name of the parameter pack is replaced by each element of the pack, in order. This isn't a clear explanation, so here are a few examples of pack expansions:

```c++
template<typename... Args>
void exampleFunc2(Args ...args){
    std::make_tuple(args...); //Expansion 1
    std::vector<size_t> v1 = {sizeof(args)...}; //Expansion 2
    std::tuple<std::vector<Args>...> t1; //Expansion 3
}
```

Suppose the function is called like this: `exampleFunc2(5, 2.0, 'c');`. Expansion 1 uses the most basic form of pack expansion. The pattern for the pack expansion is simply `args`, which is expanded to a comma separated list of all its elements. In other words, the `args` in expansion 1 gets replaced by `5, 2.0, 'c'`.

Expansion 2 expands the function parameter pack `args` with a pattern. A pattern can be thought of like a function. If you apply an operation to just the parameter pack, then when it is expanded that operation is applied to each of those elements. In expansion 2, the function can be thought of as f(x) = sizeof(x). Thus, the `sizeof(args)...` gets replaced by `sizeof(5), sizeof(2.0), sizeof('c')` in expansion 2.

Expansion 3 also uses a pattern, but this time it expands the template parameter pack `Args` instead of the function parameter pack. The pattern is `std::vector<Args>` so it gets expanded to `std::vector<int>, std::vector<double>, std::vector<char>`.

In the pack expansion `&args...` the pattern is `&args`. In `func(args)...` it's `func(args)`. If a pattern contains two parameter packs, they are expanded at the same time and must be the same length. If there is an expansion nested within another expansion, the innermost expansion occurs first.

&nbsp;

Fold expressions are essentially special types of patterns that use binary operators. The function below uses all four types of fold expressions (note that you can use any binary operator instead of the + operator):

```c++
template<int ... Vals>
std::array<int, 4> exampleFunc3(){
    return std::array<int, 4> {
        (Vals + ...), //unary right fold
        (... + Vals), //unary left fold
        (Vals + ... + 0), //binary right fold
        ( (0 + 0) + ... + Vals) //binary left fold
    };
}
```

The unary folds only have a parameter pack and an ellipsis separated by an operator (note that the surrounding parentheses are necessary), and are expanded so that the operator is placed between the elements of the parameter pack. The difference between the left and right folds is just the order in which the operators are evaluated.

For example, if `Vals` has elements e<sub>1</sub>,...,e<sub>n</sub>, then `(Vals+...)` expands to `(e1+(...+(e_{n-1}+e_n)))` and `(...+Vals)` expands to `(((e_1+ e_2)+...)+e_n)`.

Binary folds add an additional expresssion to the expansion. This expression cannot contain an unexpanded parameter pack, nor can it use an operator with a lower precedence than a cast at the top level. Basically this means that the expression cannot have something like a + in it ( `(0+0+...+Vals)` wouldn't work), but something like the subscript operator would be fine. However, you can get around this by sticking a pair of parentheses around the expression (like in the binary left fold), since only the top level operator matters.

Binary folds expand in the same way, except that the additional expression is placed in the innermost pair of parentheses. For example, `(Vals + ... + 0)` expands to `(e1+(...+(e_n+0)))`.

Suppose you called the functino like this: `exampleFunc3<>();`. This would produce a build error because the unary folds would be expanding an empty parameter pack. Only three operators can be used in a unary fold expression with a pack expansion of length 0: logical OR (`||`), logical AND (`&&`), and the comma operator (`,`). For logical OR, it evaluates to false. For logical AND, it evaluates to true. For the comma operator it evalutes to `void()`. If this behavior is undesirable, you should use the binary fold expressions and make the additional expression your desired default.