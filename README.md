# seal-wasm-embedded
WebAssembly port of SEAL C++ package released by Microsoft and containing homomorphic encryption primitives.
Generates `seal.js` only (the WebAssembly code is stored as a Base64 encoded string embedded in this .js)

## Prerequisites

### Special for Ubuntu - using [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/install-win10) :
- Enable "Windows Subsystem for Linux"
- Install [Ubuntu on Windows](https://www.microsoft.com/en-us/p/ubuntu/9nblggh4msv6?rtc=1&activetab=pivot%3Aoverviewtab)
- Install xdg-open
- Install [wslu](https://github.com/wslutilities/wslu) and config the BROWSER
```text
    sudo apt update && sudo apt upgrade
    sudo apt-get --reinstall xdg-utils

    sudo apt install apt-transport-https
    wget -O - https://api.patrickwu.space/public.key | sudo apt-key add -
    echo "deb https://apt.patrickwu.space/ stable main" | sudo tee -a /etc/apt/sources.list 

    sudo apt update
    sudo apt install wslu
    export BROWSER='wslview'
```


### EmSDK specific:
- Get the Emscripten SDK: [here](https://webassembly.org/getting-started/developers-guide/) and [here](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html)
- Use `... latest`; also you may try `... sdk-incoming-64bit binaryen-master-64bit`, but sometime it's unstable for WSL (the browser runs on Windows 10, not Ubuntu)
```text
    sudo apt update && sudo apt upgrade
    sudo apt install python2.7 python-pip
    sudo apt-get install default-jre

    git clone https://github.com/juj/emsdk.git
    cd emsdk
    ./emsdk install latest
    
    ./emsdk activate latest
    source ./emsdk_env.sh
    emcc -v
```

### Windows specific:
- Execute `emsdk_env.bat` (instead of `source ./emsdk_env.sh`) and use `emcmdprompt.bat` to open a terminal with EmSDK activated.

## Building and running
See `package.json` for the list of available `npm` commands.
For a manual build (`em++`) or run (`emrun`), inspect the corresponding .bat/.sh files.
When running in browser, also open its console - to be aware of unexpected errors, if any.
Benchmarks:
- Firefox: 10 sec.
- Chrome:  13 sec.
- Edge:    37 sec.
- Android: 96 sec. (Huawei P8 / Chrome)

### Ubuntu (build + run web server):
```text
    /emsdk  $ ./emsdk activate latest
            $ source ./emsdk_env.sh
            $ cd ...path/to/sealjs
    /sealjs $ npm i
            $ npm run ubuntu
```

### Windows 10 (build + run web server):
```text
    /emsdk  > emcmdprompt.bat
            > cd ...path/to/sealjs
    /sealjs > npm i
            > npm run windows
```

### Run in NodeJS:
```text
    /sealjs  > node .
    (or      > node index.js)
```

### Run directly in browser:
You may open `index.html` in your browser without starting the web server (e.g. `file:///D:/seal-wasm-embedded/index.html`)
