# 实验二设计

## 实验要求（用户需求）

**基于两种不同后端的词法分析单元**

功能：给定类似C语言的文法配置和一个输入串，识别串中的各个词法单元，并输出每个词法单元的类型和内容。

后端要求1：使用DFA定义文法

- 输入一个词法记号类别，包括名称、优先级
- 输入一系列DFA定义，其中每个状态机对应一个词法记号类别
- 能够识别一个输入串中的所有词法记号，并输出每个词法记号的类型和内容

后端要求2：使用FLEX工具和正则表达式定义文法

- 输入一个词法记号类别，包括名称、优先级
- 输入一系列正则表达式定义，其中每个正则表达式对应一个词法记号类别
- 调用FLEX工具对输入串进行此法分析

前端要求：

- 允许通过CLI选择不同的后端
- 输入配置文件和要匹配的字符串
- 将词法分析结果格式化输出到文件中

## 实验设计

### 总体架构

显然，两个后端对前端实现的具体功能是相同的，因此前后端之间应该通过一个统一接口进行依赖倒置。但是，注意到这两个后端的数据依赖并不相同。DFA在初始化时需要一个状态转换表，而FLEX需要一个正则表达式定义的文法。因此，前端需要根据后端的不同选择不同的配置文件格式，破坏了封装性。对于这种易变的依赖关系，一个典型的方案是使用工厂模式来解决。即前端除了词法分析器接口还拥有一个工厂接口，工厂会根据配置文件和后端选择创建加载好DFA或正则表达式定义的词法分析器实例。这样，前端只需要依赖于词法分析器接口，而不需要依赖于具体的实现类。

整体架构如下图所示：

![整体架构图]

### 词法分析器接口

定义一个词法分析器实例的功能就是在其生存周期内匹配同一种文法的字符串，所以其必须包括的接口方法很简单：

- 输入一个字符串，返回匹配有关的结果信息。

综合考虑错误处理等因素，定义一个回传的数据模型：

**单个词法单元匹配结果**
词法类别：使用std::string表示
词法内容：使用std::string表示

**匹配结果列表**
一个std::vector，包含多个此法单元匹配结果，每个结果的位置表示其匹配的顺序

**返回结果**
一个数据类，包含一个匹配结果列表，匹配是否成功的标志，错误信息

### 基于DFA的后端设计

基于DFA的词法分析器运作的大致过程如下：给定一个前缀，使用不同词法类别的DFA进行匹配，对匹配成功的类别选取最高优先级作为其最终字符串类别。对一个字符串，每次寻找前缀的时候使用最长匹配原则，直到没有可匹配的前缀为止。

可见其中主要有两部分：DFA判断程序和前缀匹配程序。其功能是分离的。DFA判断程序只负责判断一个字符串符不符合某个DFA的定义，而前缀匹配程序负责在一个字符串中寻找所有符合某个DFA的前缀。前缀匹配程序需要使用DFA判断程序来判断每个前缀是否符合DFA定义。

#### DFA判断程序

首先考虑DFA判断程序，上个实验中已经给出了一个DFA数据模型。

```cpp

struct DFA {
    std::unordered_set<char> character_set;                           // 字符集
    std::unordered_set<std::string> states_set;                       // 状态集
    std::string initial_state;                                        // 开始状态
    std::unordered_set<std::string> accepting_states;                 // 接受状态集
    std::unordered_map<std::string, std::unordered_map<char, std::string>> transitions; // 状态转换表
};

```
并且同时实现了能够判断字符串是否属于单个DFA的数据类。

```cpp

class DFASimulator {
public:
    virtual ~DFASimulator() = default;

    // 更新DFA设定
    virtual bool UpdateDFA(const DFA& dfa) = 0;

    // 模拟单一字符串
    virtual bool SimulateString(const std::string& input, std::string& simulation_log) const = 0;

    // 生成所有符合规则的字符串
    virtual std::set<std::string> GenerateAcceptedStrings(int max_length) const = 0;
};

```

因此，本次实验的DFA程序只要对其进行简单封装。

- 维护一个MAPPING表，key为词法类别，value为DFA实例
- 提供一个词法类别判断方法，输入字符串和要匹配的词法类别，返回是否匹配成功
- 提供错误处理，比如key不存在，DFA匹配出现错误等

#### 前缀匹配程序

前缀匹配程序的主要功能是对一个字符串以最长匹配原则和最高优先级原则进行匹配。其中最长匹配原则是占主导的，而最高优先级原则只在多个词法类别都匹配成功时才会使用。整个算法的自然语言描述如下：

- 维护一个空的中间状态列表。用于记录字符串中所有可能匹配成功的前缀。
- 把当前字符串初始化为整个字符串。
- 从头开始迭代当前字符串的前缀。
- 对当前前缀，把所有词法类别输入DFA匹配程序。如果有一个或多个匹配成功，取其中优先级最高的作为当前前缀的此法类别，并将（词法类别，词法内容，前缀长度）加入中间状态列表。如果没有，那么无需加入中间状态列表。
- 把下一个字符加入当前前缀，使用上一个步骤继续迭代，直到字符串结束。
- 回顾中间状态列表，选取其中最长的前缀作为最终的匹配结果，并将其加入最终结果列表。
- 将当前字符串更新为去掉前缀后的字符串，重复上面的步骤，直到没有可匹配的前缀为止。
- 返回最终结果列表。

因此可以从中抽象出几个基本的数据模型，每个包装在一个类中。

**词法类别**
名称：使用std::string表示
优先级：使用int表示，越小优先级越高
是否“多符一种”: 使用bool表示，即词法内容是否有意义，如ID的内容是变量名，IF的内容没有意义
比较方法：按照优先级大小比较（重载运算符）

**词法类别列表**
一个std::set，包含多个词法类别

**中间状态**
词法类别：使用std::string表示
词法内容：使用std::string表示
前缀长度：使用int表示
前缀优先级：使用int表示，越小优先级越高
比较方法：按照前缀长度大小比较（重载运算符）

**中间状态列表**
一个std::set，包含多个中间状态

**匹配结果**
词法类别：使用std::string表示
词法内容：使用std::string表示

**最终结果**
一个std::vector，包含多个匹配结果

此处最终结果和要求返回的数据类中的匹配结果列表是相同的，因此可以直接使用最终结果作为返回结果。有很多可以优化的部分，但考虑到可读性和程序本身轻量级，暂时不做优化。

#### 初始化

根据以上需求，定义一个专门用于初始化的数据类：

```cpp
struct ConstructionInfo
{
    std::vector<TokenType> token_types; // List of token types
    std::vector<DFA> dfa_configurations; // List of DFA configurations
};
```

### 基于FLEX的后端设计

最初的设想是使用FLEX的C++接口来实现FLEX后端，但是其无法直接在运行时加载正则表达式定义的文法。经过资料查找发现，参考FLEX设计的现代化正则表达式处理库RE/FLEX具备此功能，因此决定使用RE/FLEX来实现FLEX后端。其实先与DFA后端几乎一致，只是将DFA判断程序替换为RE/FLEX的正则表达式判断程序。

### 后端工厂类设计

该部分主要解决的问题是：
- 根据后端类型创建不同的词法分析器实例
- 根据后端类型加载不同格式的配置文件

#### DFA后端数据模型设计

根据DFA后端的设计，配置文件中需要包含以下信息：

- 词法类别列表，其中包括每个类别的名称和优先级
- DFA列表，其中包括每个DFA的状态集、字符集、初始状态、接受状态集和状态转换表
- 词法类别和DFA的映射关系

同时注意到词法类别列表对FLEX后端也是有用的，因此可以将其分离在一个单独的配置文件中。使用YAML格式进行配置，方便后续扩展，一个典型的配置示例如下：

```yaml
token_types:
  - name: "IDENTIFIER"
    priority: 1
    has_content: true
  - name: "NUMBER"
    priority: 2
    has_content: true
  - name: "OPERATOR"
    priority: 3
    has_content: false # 假设操作符没有内容
```

对于DFA定义，在实验一中已经设计过了一个DFA数据模型，但是其字符集表示占用了大量空间。考虑到经典DFA的字符集由单个字符组成，因此可以将其简化为一个字符串表示。一个典型的DFA配置文件如下：

```yaml
dfas:
  - name: "IDENTIFIER"
    character_set: "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_"
    states_set: ["q0", "q1"]
    initial_state: "q0"
    accepting_states: ["q1"]
    transitions:
      - from: "q0"
        to: "q1"
        character: "abcde"
      - from: "q1"
        to: "q1"
        character: "Z"
```
#### FLEX后端数据模型设计

相比与DFA后端，FLEX所需的配置文件信息更少。只需要包含词法类别列表和正则表达式列表即可。正则表达式列表中每个正则表达式对应一个词法类别，正则表达式的优先级由其在列表中的位置决定。一个典型的FLEX配置文件如下：

```yaml
token_types:
  - name: "IDENTIFIER"
    priority: 1
    has_content: true
  - name: "NUMBER"
    priority: 2
    has_content: true
  - name: "OPERATOR"
    priority: 3
    has_content: false # 假设操作符没有内容

regexps:
- regexp: "^[a-z]+$"
  token_type: "IDENTIFIER"
- regexp: "^[0-9]+$"
  token_type: "NUMBER"
- regexp: "D"
  token_type: "OPERATOR"
```