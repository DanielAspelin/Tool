class INVENTORY {
public:
    static constexpr ITEM entries[] {
        {"human","grammar","overview","Human grammar","Grammar describes how a language organizes expressions. Compare morphology, syntax, agreement and word order within a specific language.","https://wals.info/",DATATYPE::reference},
        {"human","grammar","syntax","Syntax","Syntax describes how words and phrases combine. Example: the noun phrase ‘the red book’ contains a determiner, modifier and head noun in English.","https://wals.info/",DATATYPE::example},
        {"human","grammar","morphology","Morphology","Morphology studies word structure. Finnish talo-ssa means ‘in the house’: talo is the stem and -ssa is an inessive ending.","https://www.kotus.fi/en/",DATATYPE::example},
        {"human","grammar","phonology","Phonology","Phonology describes the organization of speech sounds or other contrastive forms. PHOIBLE indexes documented segment inventories.","https://phoible.org/",DATATYPE::reference},
        {"human","grammar","semantics","Semantics","Semantics studies meaning. The phrase ‘every guest read a book’ can allow each guest to read a different book.","",DATATYPE::example},
        {"programming","python","syntax","Python syntax","Indentation forms blocks. This example defines a function and returns a value.\n\ndef square(value):\n    return value * value","https://docs.python.org/3/reference/",DATATYPE::example},
        {"programming","cpp","syntax","C++ syntax","This function takes a typed parameter and returns a typed result.\n\nint square(int value) {\n    return value * value;\n}","https://eel.is/c++draft/",DATATYPE::example},
        {"programming","javascript","syntax","JavaScript syntax","This function expression takes one argument.\n\nconst square = value => value * value;","https://tc39.es/ecma262/",DATATYPE::example},
        {"machine","wasm","syntax","WebAssembly text format","A function adding two i32 values.\n\n(func (param i32 i32) (result i32)\n  local.get 0\n  local.get 1\n  i32.add)","https://www.w3.org/TR/wasm-core-2/",DATATYPE::example},
    };
};
