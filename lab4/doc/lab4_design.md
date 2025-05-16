# 实验四设计

## 实验要求（用户需求）

目标：在实验三的基础上，对ParsingTable进行SLR(1)冲突分析，生成SLR(1)分析表。

输入：实验三的LR(0)规范族、文法符号的FOLLOW集

输出：SLR(1)分析表（ACTION/GOTO二维表）
SLR（1）分析引入了 FOLLOW 集 来辅助解决部分冲突。在遇到移进 - 归约或归约 - 归约冲突时，通过检查归约项目左部非终结符的 FOLLOW 集与当前输入符号的关系来确定操作。

## 实验设计

### 需求分析

显然该部分是语法分析器的一部分，其目的仍然是生成一个ParsingTable。只是在生成过程中添加了SLR(1)分析来尝试排除冲突。因此应当保持原有的程序不变，在语法分析接口中派生一个新的类SLRParser。

SLR冲突解决大致可以分为三个部分：

1. 计算CFG文法的FOLLOW集。
2. 在ParsingTable中找到所有的冲突。
3. 对每个冲突，尝试参考FOLLOW集来解决冲突。
4. 如果冲突解决全部成功，更新ParsingTable，返回成功；否则返回失败。

从中可以抽象出几个单一职能模块：
1. CFG文法分析器：计算一个给定文法的FIRST集和FOLLOW集。
2. ParsingTable冲突检查工具：检查并记录ParsingTable中的冲突。
3. SLR冲突解决器：尝试解决ParsingTable中的冲突。
4. SLR分析器：将上述三个模块结合起来，完成SLR(1)分析表的生成。

### 关键数据模型设计

#### CFG文法分析器
该模块的输入是CFG文法，输出是FIRST集和FOLLOW集。对于FIRST集，可以使用一个从`cfg_model::symbol`指向`unordered_set<symbol>`的`unordered_map`表示。此外，由于设计时不显式表示空符号，因此需要添加一个`unordered_set<symbol>`来表示FIRST集中拥有空符号的非终结符。

对于FOLLOW集，除了使用`unordered_map`表示外，还需要记录CFG文法的结束符号。

#### ParsingTable冲突检查工具
考虑到该工具对词法分析很常用，将其作为成员函数集成在`ParsingTable`类中。该工具的输入是ParsingTable，输出是冲突列表。冲突列表可以使用一个`unordered_map<symbol, string>`表示，表示在相应的表格中存在冲突。

#### SLR冲突解决器
冲突解决需要以下几个方面的信息：
1. ParsingTable
2. 对应的CFG文法，用于计算FIRST集和FOLLOW集
3. 每个ParsingTable项对应的LR(0)项目集合

对于前两者，可以直接传入；对于第三者，需要设计一个从`ParsingTable`到LR(0)项目集合的映射。可以使用一个`unordered_map<string, unordered_map<symbol, unordered_set<shared_ptr<Item>>>>`表示。该映射的key为ParsingTable项的字符串表示，value为该项对应的LR(0)项目集合。由于LR(0)项目集合是一个集合，因此可以使用`unordered_set<shared_ptr<Item>>`表示。特别的，这个映射应该在ParsingTable构造时一同产生。


