
ESP-IDF 的项目工程与 Keil 的传统工程模式有显著不同。Keil 工程通常依赖于特定的项目文件格式（如 .uvprojx 或 .uvoptx），这些文件记录了项目的结构、配置和依赖，且需要通过 Keil 软件进行管理和编译。而 ESP-IDF 则采用更灵活的构建方式，ESP-IDF 的项目只是一个特定的文件夹，其中包含了构建可执行应用程序所需的全部文件和配置，以及其他支持型文件，例如分区表、数据/文件系统分区和引导程序。这些文件描述了项目的源码路径、依赖关系和编译选项，而不是依赖于 IDE 专属的配置格式。这样可以方便地在不同的开发环境中切换。

关于ESP-IDF工程结构可以参考官方文档：[示例项目](https://docs.espressif.com/projects/esp-idf/zh_CN/v5.3.2/esp32/api-guides/build-system.html#example-project-structure)

这部分没有什么新的东西，只是需要明白ESP-IDF所规定的项目框架，下面的内容都提炼自官方文档，大家也可以直接前往官方文档查看。
### 项目目录

esp-idf的标准目录如下所示：

```c
- myProject/
             - CMakeLists.txt
             - sdkconfig
             - components/ - component1/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                           - component2/ - CMakeLists.txt
                                         - Kconfig
                                         - src1.c
                                         - include/ - component2.h
             - main/       - CMakeLists.txt
                           - src1.c
                           - src2.c
             - build/
```


-  `CMakeLists.txt` 文件，这是 CMake 用于学习如何构建项目的主要文件，可以在这个文件中设置项目全局的 CMake 变量。顶层项目 CMakeLists.txt 文件会导入/tools/cmake/project.cmake 文件，由它负责实现构建系统的其余部分。该文件最后会设置项目的名称，并定义该项目。
  
- `sdkconfig` 项目配置文件，执行 idf.py menuconfig 时会创建或更新此文件，文件中保存了项目中所有组件（包括 ESP-IDF 本身）的配置信息。 sdkconfig 文件可能会也可能不会被添加到项目的源码管理系统中。
  
- 可选的 `components` 目录中包含了项目的部分自定义组件，并不是每个项目都需要这种自定义组件，但它有助于构建可复用的代码或者导入第三方（不属于 ESP-IDF）的组件。或者，您也可以在顶层 CMakeLists.txt 中设置 EXTRA_COMPONENT_DIRS 变量以查找其他指定位置处的组件。有关详细信息，请参阅 重命名 main 组件。如果项目中源文件较多，建议将其归于组件中，而不是全部放在 “main” 中
  
- `main` 目录是一个特殊的组件，它包含项目本身的源代码。”main” 是默认名称，CMake 变量 COMPONENT_DIRS 默认包含此组件，但您可以修改此变量。
  
- `build` 目录是存放构建输出的地方，如果没有此目录，idf.py 会自动创建。CMake 会配置项目，并在此目录下生成临时的构建文件。随后，在主构建进程的运行期间，该目录还会保存临时目标文件、库文件以及最终输出的二进制文件。此目录通常不会添加到项目的源码管理系统中，也不会随项目源码一同发布。

### 项目 CMakeList.txt 文件

每个项目都有一个顶层 `CMakeLists.txt` 文件，包含整个项目的构建设置。默认情况下，项目 CMakeLists 文件会非常小。

``` CMake
cmake_minimum_required(VERSION 3.16)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(teat)
```

> `cmake_minimum_required(VERSION 3.5)` 必须放在 CMakeLists.txt 文件的第一行，它会告诉 CMake 构建该项目所需要的最小版本号。ESP-IDF 支持 CMake 3.5 或更高的版本。
   `include($ENV{IDF_PATH}/tools/cmake/project.cmake)` 会导入 CMake 的其余功能来完成配置项目、检索组件等任务。
   `project(teat)` 会创建项目本身，括号内容作为项目名称。

上面的内容为 项目**必要部分**，除此之外，还有一些 **可选部分**。

- `COMPONENT_DIRS`：组件的搜索目录，默认为 `IDF_PATH/components`、 `工程文件夹/components`、和变量 `EXTRA_COMPONENT_DIRS`。如果您不想在这些位置搜索组件，请覆盖此变量。(通常情况下此变量不需要覆盖，如果想要添加额外的组件，请放到) `工程文件夹/components`或者变`EXTRA_COMPONENT_DIRS`中
- `EXTRA_COMPONENT_DIRS`：用于搜索组件的其它可选目录列表。路径可以是相对于项目目录的相对路径，也可以是绝对路径。
- `COMPONENTS`：要构建进项目中的组件名称列表，默认为 `COMPONENT_DIRS` 目录下检索到的所有组件。使用此变量可以“精简”项目以缩短构建时间。请注意，如果一个组件通过 `COMPONENT_REQUIRES` 指定了它依赖的另一个组件，则会自动将其添加到 `COMPONENTS` 中，所以 `COMPONENTS` 列表可能会非常短。
以上变量中的路径可以是绝对路径，或者是相对于项目目录的相对路径。
要使用 cmake 中的 `set()` 命令 来设置这些变量。  如 `set(VARIABLE "VALUE")`。

### 组件 CMakeList.txt 文件

组件是 COMPONENT_DIRS 列表中的任何目录。此列表中的目录可以是组件自身（即包含 CMakeLists.txt 文件的目录），也可以是子目录为组件的顶级目录，例如`$ENV{PROJECT_DIR}/components`

`构建` 或 `编译` 时，ESP-IDF 搜索所有待构建的组件的顺序为`COMPONENT_DIRS` 指定的顺序依次进行。  
因此，在默认情况下：

1. 搜索 ESP-IDF 内部组件（`$ENV{IDF_PATH}/components`）
2. 搜索 `EXTRA_COMPONENT_DIRS` 中的组件
3. 搜索项目组件（`$ENV{PROJECT_DIR}/components`）

> **注意**  :
   如果这些目录中的两个或者多个包含具有相同名字的组件，则使用 **搜索到的最后一个** 位置的组件。这就允许将组件复制到项目目录中再修改以覆盖 ESP-IDF 组件。
   

组件的最小的CMakeList.txt文件只有一句:

``` CMake
idf_component_register(SRCS "my_component.c"
                    INCLUDE_DIRS "include")
```

- `SRCS` 是源文件列表（_.c、_.cpp、_.cc、_.S），里面所有的源文件都将会编译进组件库中。
- `INCLUDE_DIRS` 是目录列表，里面的路径会被添加到所有需要该组件的组件（包括 main 组件）全局 include 搜索路径中。

除此之外，还有很多非必须的参数:

- `REQUIRES` 声明组件的public依赖，可供其他依赖者访问
- `PRIV_REQUIRES`声明private依赖，只能被当前组件访问。

# 参考链接

1. https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/build-system.html