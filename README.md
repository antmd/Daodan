Daodan
======

## Table of Contents
* [License](./LICENSE)
* [What is this?](#what-is-this)

***

### What is this?
Daodan is an assisted code injection library. This means that to do full code injection or over-rides, the Daodan library must be a dependency of the project you wish to inject code into. If it isn't a dependent library, then it is still possible to add undefined symbols to hook into your application or project that you want Daodan to inject into. If source access to the project you wish to inject into isn't possible, then this library will let you inject code into the process and you will have to setup your own runloop to manage calling any code along-side the application. Included is part of a symbol table lookup library that will allow you to find and call functions that are inside of the application's symbol table.

By default, Daodan will load the symbol table of the application it is injected into, however it is possible to load up the symbol tables of any other linked library.