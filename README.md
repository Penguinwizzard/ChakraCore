# ChakraCore

ChakraCore is the core part of the engine that powers Microsoft Edge.  ChakraCore supports Just-in-time (JIT) compilation of JavaScript for x86/x64/ARM, garbage collection, and a wide range of the latest JavaScript features.  ChakraCore also supports the JavaScript Runtime (JSRT) API, which allows you to easily embed ChakraCore in your applications.

You can stay up-to-date on progress by following the [Edge developer blog](http://blogs.windows.com/msedgedev/).

## Building ChakraCore

In order to build ChakraCore, you will need [Visual Studio](https://www.visualstudio.com/products/visual-studio-community-vs) 2013 or 2015 with C++ support installed.  Once you have Visual Studio installed:

* Open `Build\Chakra.Core.sln` in Visual Studio
* Build Solution

More details in [Building ChakraCore](https://github.com/Microsoft/ChakraCore/wiki/Building-ChakraCore).

## Using ChakraCore

Once built, you have a few options for how you can use ChakraCore:

* The most basic is to test the engine is running correctly with the *ch.exe* binary.  This app is a lightweight hosting of JSRT that you can run small applications.  After building, you can find this binary in: `Build\VcBuild\bin\[platform+output]`  (eg. `Build\VcBuild\bin\x64_debug`)
* You can [embed ChakraCore](https://github.com/Microsoft/ChakraCore/wiki/Embedding-ChakraCore) in your applications.  There are samples and documentation available on how to do this.
* Finally, you can also use ChakraCore as the JavaScript engine in Node.  You can learn more by reading how to use [Chakra as Node's JS engine](https://github.com/Microsoft/node)

_A note about using ChakraCore_: ChakraCore is the foundational JavaScript engine, but it does not include the external APIs that make up the modern JavaScript development experience.  For example, DOM APIs like ```document.write()``` are additional APIs that are not available by default and would need to be provided.  For debugging, you may instead want to use ```print()```.

## Security

If you believe you have found a security issue in ChakraCore, please share it with us privately following the guidance at the Microsoft [Security TechCenter](https://technet.microsoft.com/en-us/security/ff852094). Reporting it via this channel helps minimize risk to projects built with ChakraCore.

## Contribute

There are many ways to contribute to ChakraCore.

* [Submit bugs](https://github.com/Microsoft/ChakraCore/issues) and help us verify fixes
* [Submit pull requests](https://github.com/Microsoft/ChakraCore/pulls) for bug fixes and features and discuss existing proposals
* Chat about [#ChakraCore](https://twitter.com/search?src=typd&q=ChakraCore) on Twitter

Please refer to [Contribution guidelines](https://github.com/Microsoft/ChakraCore/wiki/Contributor-Guidance) for more details.

## Roadmap
For details on our planned features and future direction please refer to our [roadmap](https://github.com/Microsoft/ChakraCore/wiki/Roadmap).

## Documentation

* [Quickstart Embedding ChakraCore](https://github.com/Microsoft/ChakraCore/wiki/Embedding-ChakraCore)
* [JSRT Reference](https://github.com/Microsoft/ChakraCore/wiki/JavaScript-Runtime-%28JSRT%29-Reference)
* [Architecture overview](https://github.com/Microsoft/ChakraCore/wiki/Architecture-Overview)
* [Contribution guidelines](https://github.com/Microsoft/ChakraCore/wiki/Contributor-Guidance)
* [Other resources](https://github.com/Microsoft/ChakraCore/wiki/Resources)
