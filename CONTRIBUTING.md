# Contributing to Xenon

Everyone is welcome to contribute to Xenon and every contribution is appreciated, although there are some rules to keep in mind.

## Rules

- Do not use AI do write the code for you. Pull requests which I think are vibecoded will not be accepted.
- Please attempt to write relatively good and minimal code. Xenon is meant to be lightweight, not bloated.
- Taking inspiration from other open source projects is allowed. Copying it line for line isn't.
- Throughly test whatever feature you added or whatever bug you fixed.
- Add comments to code which is not as straightforward.
- Do not make changes without an explanation unless it's obvious.

## Formatting

Keeping the code clean and consistent is important. That is why i ask you to follow these formatting rules.

```cpp

// Single line comments

/*
Multi
line
comments
*/

// CORRECT
int add(int a, int b)
{
    return a + b;
}

// WRONG
int add(int a,int b){
    return a+b;
}

```