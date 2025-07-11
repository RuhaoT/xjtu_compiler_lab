# 实验三设计

## 实验要求（用户需求）

目标：实现一个经典语法分析器的前导组件，构建给定输入文法的LR(0)项目集规范族（Canonical Collection of LR(0) Items），为后续生成SLR(1)分析表提供基础。

要求：
- 输入文法规则，支持扩展巴科斯范式（如 E → E + T | T）。
- 需要手动增广文法：添加新的起始产生式 S' → S（S为原文法起始符号）。
- 要求生成LR(0)项目集规范族：计算每个状态的闭包（Closure）和状态转- 移（Goto）。
- 输出所有项目集及其内核项（Kernel Items）。
- 规范族的图形化表示（如状态编号和转移关系）。
- 需要考虑是否属于LR(0)文法（无移进-归约或归约-归约冲突）

## 实验设计

### 需求分析

可以将用户的需求大致分为三个部分：
1. 输入文法规则
2. 计算LR(0)项目集规范族
3. 输出项目规范族
下面尝试将这些需求拆分为单一职能的业务模块。

输入文法规则：
- 需要定义一种存储格式（计划使用YAML文件）对文法进行描述。
- 需要定义一个中间数据模型，在程序中对文法进行存储和操作。
- 需要实现一个解析器，将YAML文件解析为中间数据模型。
- 需要在输入后对文法进行合法性检查，确保文法符合LR(0)的要求。

计算LR(0)项目集规范族：
- 需要一个项目集合生成装置，增广CFG文法，并生成LR(0)项目集itemset。
- 需要定义一个NFA数据模型，并实现NFA向DFA的转换（闭包计算），以利用先前实验中实现的DFA模拟器。同时需要考虑到NFA和DFA之间的状态对应关系。
- 需要实现一个规范族生成器，该生成器接受文法定义和itemset作为输入，调用NFA模拟器生成itemNFA和对应的itemDFA。然后根据原始文法和DFA的关系，参考DFA生成LR(0)项目集规范族。

输出项目规范族：
- 需要根据状态转移关系判断是否为LR(0)文法。
- 需要输出生成的项目集，并标出其中的内核项。
- 需要将生成的项目集规范族进行图形化表示，如表示成一张状态转移图。

### 整体架构

在需求分析中确定的三个模块职能是单一的。因此可以设计一个总体控制程序作为前端，其保留有三个接口来倒置依赖于这三个模块。在执行时先调用输入模块接口获得文法定义，接着调用计算模块接口生成LR(0)项目集规范族，最后调用输出模块接口输出项目集规范族。这样可以将三个模块的实现细节封装在各自的模块中，前端只需要关注接口的使用。

![整体架构图]()

如图所示，CFGLoader为输入模块接口，并由此派生实现YAMLCFGLoader类用于读取输入。

LR0ParsingTableGenerator为计算模块接口，并由此派生实现SimpleLR0ParsingTableGenerator类用于生成LR(0)项目集规范族。其由两个独立类聚合：ItemSetGenerator和ItemSet2ParsingTable。前者负责将输入文法转换成一个ItemSet，后者负责从ItemSet生成LR(0)项目集规范族。其中ItemSet2ParsingTable类的实现依赖于NFA2DFAConverter类和DFASimulator类。

### 关键数据模型设计

为了确定接口即后续程序设计的实现细节。首先对一些整个程序共有的关键数据模型进行设计。

其中包括：
- CFG文法规则
- LR(0)项目集ItemSet
- NFA
- LR(0)项目集规范族ParsingTable

此外，还需要设计一个YAML文件格式，用于存储和加载CFG文法规则。

#### CFG文法规则

一个上下文无关文法包含四个部分
- 终结符号（类别）集合，对本实验只要划分出类别即可，具体符号和类别的判断由词法分析器提供。
- 非终结符号集合。
- 产生式集合。
- 一个起始符号。

为了程序效率和可读性，作如下约定：
- 每个CFG的符号都有两个属性：符号名称和符号类别，符号名称使用字符串表示，符号类别使用bool表示是否为终结符号。
- 尽管有显式符号类别，依然规定终结符号的名称不能和非终结符号的名称相同。

在CFG模型中，使用unordered_set来存储中介符号和非终结符号集合，并使用unordered_map来存储产生式集合。产生式集合的key为非终结符号，value为一个vector，vector中存储该非终结符号的所有产生式。每个产生式使用vector表示，vector中的元素为符号名称。

对于一个产生式（此处指没有经过化简，右侧只有单一候选式的产生式），其左边是一个非终结符号，右边是一个符号序列。符号序列使用vector表示，而产生式集合使用unordered_map存储，其中key为非终结符号，value为一个vector，vector中存储该非终结符号的所有产生式。

此外还有一点需要注意，那就是CFG文法中的空产生式。为了清晰期间，我们将空产生式和非空产生式单独存储，即产生式集合分为一个unordered_map和一个unordered_set。unordered_map存储非空产生式，unordered_set存储可以为空的产生式左侧符号。

#### LR(0)项目集ItemSet

ParsingTableGenerator产生规范族的第一步是将输入的CFG文法进行扩展，产生其对应的项目集合。

项目集合中的每个项目形式和产生式相似，但是引入了一个点符号（.）来表示当前分析的进度。为了更清晰的表示其含义，我们将一个Item定义为一个包含以下部分的结构体：
- 产生式左侧符号
- 已经分析的符号序列，存储在vector中
- 尚未分析的符号序列，存储在vector中

因此ItemSet可以看作是一个Item的集合。为了解耦ItemSet和Item的关系，ItemSet使用unordered_set来存储Item集合。

除了Item集合之外，ItemSet还需要标出初始状态（此处指增广后文法的初始状态）对应的两条产生式：未分析的产生式和已经分析的产生式。这两条产生式将在后续的LR0分析下推自动机中标记初始状态和最终接受状态。

#### NFA

考虑到单一职责原则，此处的NFA是一个通用NFA模型，并不包含LR(0)项目集的相关信息。

不同于DFA，NFA的转移涉及到ε-转移。为了清晰期间，我们将非空转移和空转移分别存储，因此一个NFA包含以下部分：
- 状态集合: `unordered_set<string>`
- 字符集合：`unordered_set<char>`
- 起始状态: `string`
- 接受状态集合: `unordered_set<string>`
- 非空转移函数: `unordered_map<string, unordered_map<char, unordered_set<string>>>`
- 空转移函数: `unordered_map<string, unordered_set<string>>`

#### LR(0)项目集规范族ParsingTable

LR(0)项目集规范族ParsingTable是LR(0)分析下推自动机的核心部分。其主要包含ACTION表和GOTO表。ACTION表用于存储状态转移信息，GOTO表用于存储状态转移的目标状态。事实上，ACTION表和GOTO表只需要额外记录一份CFG的符号/状态表即可。

在ACTION表中，每个表格的值（假设没有冲突）可能拥有以下几种不同的涵义：
- 移进（Shift）：表示当前状态可以转移到下一个状态。
- 归约（Reduce）：表示当前状态可以归约为一个产生式。
- 接受（Accept）：表示当前状态可以接受输入串。
- 空（Empty）：表示当前状态没有任何转移。

因此我们可以将一个ACTION如下表示：
- 动作类型: `string`
- 目标状态: `string`
- 产生式: `pair<symbol, vector<symbol>>`

因此ACTION表可以表示为一个`unordered_map<string, unordered_map<symbol>, unordered_set<Action>>>`，其中key为当前状态，value为一个unordered_map，key为输入符号。考虑到冲突的情况，value为一个unordered_set，存储所有可能的动作。
GOTO表的表示方式和ACTION表类似，由当前状态和输入状态确定一个unordered_set，存储所有可能的转移状态：`unordered_map<string, unordered_map<symbol, unordered_set<string>>>`。

因此一个ParsingTable可以表示为一个包含ACTION表、GOTO表和CFG符号表的结构体。





#### YAML文件格式

YAML文件格式的设计主要是为了方便用户输入文法规则。同样，其中应该涵盖CFG文法规则的各个部分。一个典型的YAML文件示例如下：

```YAML
# S' → E
# E → E + T | T
# T → T * F | F
# F → (E) | id
cfg:
  terminals:
  - "id"
  - "+"
  - "*"
  - "("
  - ")"
  non_terminals:
  - "E"
  - "T"
  - "F"
  initial_symbol: "E"
  production_rules:
  - lhs: "E"
    rhs:
    - "E"
    - "+"
    - "T"
  - lhs: "E"
    rhs:
    - "T"
  - lhs: "T"
    rhs:
    - "T"
    - "*"
    - "F"
  - lhs: "T"
    rhs:
    - "F"
  - lhs: "F"
    rhs:
    - "("
    - "E"
    - ")"
  - lhs: "F"
    rhs:
    - "id"
```

特别的，如果在rhs中提供一个空串，则表示该产生式可以为空。此时需要在YAML文件中提供一个空串的标识符（如`""`）。

```YAML
  # E → ε
  - lhs: "E"
    rhs:
    - ""
```

## 实现细节

### 1. 输入模块

输入模块只提供了一个单一的功能接口：给出一个文件的路径，返回一个CFG文法规则对象。其工作过程大致分为四个步骤：
1. 检查YAML文件的合法性：文件是否存在、所定义的各个格式字段是否存在等。
2. 解析YAML文件：将YAML文件解析为一个CFG文法规则对象。
3. 检查CFG文法规则的合法性：检查终结符号和非终结符号是否有重复、产生式是否符合LR(0)文法的要求等。
4. 返回CFG文法规则对象。

### 2. 项目集与规范族产生器

#### 2.1 项目集产生器

项目产生器的主要目标是给出一个CFG文法相对应的ItemSet。其自然语言过程描述如下：
1. 读取CFG并确定初始状态。
2. 生成一个新状态名，这个状态名应该和所有的状态名不冲突。
3. 增广CFG文法，添加新的起始产生式 S' → S。
4. 对CFG中的每个产生式，构建其所有文法项目。
5. 生成项目集ItemSet。

#### 2.2 规范族产生器

规范族产生器接收一个ItemSet对象，其主要功能如下：
1. 创建一个与ItemSet相对应的NFA对象，同时记录ItemSet与NFA之间的状态映射关系。
2. 调用NFA-DFA转换器，将NFA转换为DFA，同时得到NFA和DFA之间的状态映射关系。
3. 计算得到ItemSet和DFA之间的状态映射关系。
4. 生成LR(0)项目集规范族ParsingTable对象。

### 3. NFA-DFA转换器

NFA-DFA转换器的目前是将一个NFA转换为一个等价的DFA。此处的NFA是一个通用的NFA模型，并不包含LR(0)项目集的相关信息。其工作过程大致分为以下几个步骤：
1. 读取NFA的状态集合、字符集合、起始状态和接受状态集合。
2. 将NFA中的状态按照ε-闭包进行分组，作为将来DFA状态的预备集合，同时进行重命名、判断是否为接受状态等。
3. 在NFA状态和DFA状态之间建立映射关系。
4. 根据NFA定义和其与DFA状态的映射关系，生成DFA的转移函数。
5. 根据闭包分组情况，确定DFA的起始和接受状态集合。
6. 构建并返回DFA对象。