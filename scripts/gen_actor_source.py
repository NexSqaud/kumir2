#!/usr/bin/python
#coding=utf-8

import sys
import json
import string

update = "--update" in sys.argv
project = "--project" in sys.argv
help = "--help" in sys.argv

if help or len(sys.argv)<3:
    sys.stderr.write("""
Usage:
    %s --project|--update DECLARATION.json

    --project       Run in generate project mode
    --update        Run in update files mode (used from Makefiles)

Example (create new actor skeleton):
    cd kumir2/src/actors
    mkdir mygreatactor
    cd mygreatactor
    { ... create here file mygreatactor.json ...}
    ../../../scripts/gen_actor_source.py --project mygreatactor.json

This will create the following files:
    MyGreatActor.pluginspec     -- actor plugin spec
    CMakeLists.txt              -- CMake project file
    mygreatactormodule.h        -- header skeleton
    mygreatactormodule.cpp      -- source skeleton

""" % sys.argv[0])
    sys.exit(0)

f = open(sys.argv[-1], 'r')
actor = json.load(f)
f.close()

def name_to_cpp(name, firstUpper = False):
    assert type(name)==dict or type(name)==str or type(name)==unicode
    if type(name)==dict:
        ascii = name["ascii"]
    else:
        ascii = name
    newName = ""
    nextIsCap = firstUpper
    if ascii=="+":
        return "OperatorPLUS"
    elif ascii=="-":
        return "OperatorMINUS"
    elif ascii=="*":
        return "OperatorASTERISK"
    elif ascii=="**":
        return "OperatorPOWER"
    elif ascii=="/":
        return "OperatorSLASH"
    elif ascii=="=":
        return "OperatorEQUAL"
    elif ascii=="<":
        return "OperatorLESS"
    elif ascii==">":
        return "OperatorGREATER"
    elif ascii.startswith(":="):
        return "OperatorASSIGN"
    for c in ascii:
        if c==' ':
            nextIsCap = True
        else:
            if nextIsCap: 
                newName += c.upper()
            elif c.upper() in "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890_": 
                newName += c
            nextIsCap = False
    newName = newName.replace("#","").replace("?","").replace("!","")
    return newName

def basetype_to_qtype(baseType):
    if baseType=="string":
        return "QString"
    elif baseType=="double":
        return "qreal"
    elif baseType=="char":
        return "QChar"
    elif "types" in actor.keys():
        types = actor["types"]
        for tp in types:
            name = tp["name"]
            if type(name)==dict:
                name = name["ascii"]
            if name==baseType:
                typename = tp["cppdecl"]
                if typename.startswith("struct "):
                    p = typename.index("{")
                    typename = typename[7:p]
                if typename.startswith("class "):
                    p = typename.index("{")
                    typename = typename[6:p]
                return typename

    return baseType

def vector_type(baseType): return "QVector< "+baseType+" >"

def arg_to_qarg(arg):
    try:
        dim = int(arg["dim"])
    except:
        dim = 0
    qtype = basetype_to_qtype(arg["baseType"])
    for i in range(0,dim): qtype = vector_type(qtype)
    try:
        access = arg["access"]
    except:
        access = "in"
    if access=="in":
        qtype = "const "+qtype
    if "out" in access or not arg["baseType"] in ["int", "bool", "double"]:
        qtype += "&"
    return qtype+" "+name_to_cpp(arg["name"])

# --------------------------- base class to be subclassed and implemented

actorCPPNameSpace = "Actor" + name_to_cpp(actor["name"], firstUpper=True)
actorCPPModuleName = name_to_cpp(actor["name"], firstUpper=True)+"Module"

hName = actorCPPModuleName.lower()+"base.h"
cppName = actorCPPModuleName.lower()+"base.cpp"

handlesCustomTypeConstant = "false"
if "types" in actor.keys():
    for tp in actor["types"]:
        if "constants" in tp and (tp["constants"]==True or tp["constants"]==1):
            handlesCustomTypeConstant = "true"
            break

parseCustomTypeConstantBody = "    Q_UNUSED(typeName); Q_UNUSED(literal);\n"
parseCustomConstantH = ""
parseCustomConstantHB = ""
parseCustomConstantCPP = ""

if handlesCustomTypeConstant:
    firstIf = True
    parseCustomTypeConstantBody = ""
    for tp in actor["types"]:
        typename = tp["cppdecl"]
        if typename.startswith("struct "):
            p = typename.index("{")
            typename = typename[7:p]
        if typename.startswith("class "):
            p = typename.index("{")
            typename = typename[6:p]
        typename = typename.strip()
        if "constants" in tp.keys() and (tp["constants"]==True or tp["constants"]==1):
            methodName = "parseConstantOfType_"+typename
        if firstIf:
            firstIf = False
            parseCustomTypeConstantBody += "if "
        else:
            parseCustomTypeConstantBody += "    else if "

        parseCustomTypeConstantBody += "( literal==QString::fromAscii(\"%s\") ) {\n" % typename
        parseCustomTypeConstantBody += "        %s value;\n" % typename
        parseCustomTypeConstantBody += "        if ( m_module->%s(literal, value) ) {\n" % methodName
        parseCustomTypeConstantBody += "            char data[sizeof(%s)];\n" % typename
        parseCustomTypeConstantBody += "            memcpy(data, &value, sizeof(value));\n"
        parseCustomTypeConstantBody += "            result = QByteArray(data);\n"
        parseCustomTypeConstantBody += "        }\n"
        parseCustomTypeConstantBody += "    }\n"
        parseCustomConstantH += "    bool "+methodName+"(const QString & literal, "+typename+" &value);\n"
        parseCustomConstantHB += "    virtual bool "+methodName+"(const QString & literal, "+typename+" &value) = 0;\n"
        parseCustomConstantCPP += "bool "+actorCPPModuleName+"::"+methodName+"(const QString & literal, "+typename+" &value) {\n    //TODO implement me\n    return false;\n}\n"


DO_NOT_EDIT = """/*
DO NOT EDIT THIS FILE!

This file is autogenerated from "%s" and will be replaced
every build time

*/
""" % sys.argv[1]

if update: 
    h = open(hName, "w")
    h.write(DO_NOT_EDIT)
    h.write("#ifndef "+hName.replace(".","_").upper()+"\n")
    h.write("#define "+hName.replace(".","_").upper()+"\n")
    h.write("#include <QtCore>\n#include <QtGui>\n\n")
    h.write("#include \"extensionsystem/kplugin.h\"\n\n")


    h.write("namespace %s {\n\n" % actorCPPNameSpace)
    
    if "types" in actor.keys():
        for tp in actor["types"]:
            if tp["cppdecl"].startswith("struct") or tp["cppdecl"].startswith("class"):
                h.write(tp["cppdecl"]+";\n\n")
    
    h.write("class %sBase\n    : public QObject\n{\n" % actorCPPModuleName)
    h.write("    Q_OBJECT\n")
    h.write("public:\n")
    h.write("    %sBase(ExtensionSystem::KPlugin * parent);\n" % actorCPPModuleName)
    h.write(parseCustomConstantHB)
    h.write("public slots:\n")
    h.write("    // Reset actor state before program starts\n")
    h.write("    virtual void reset() = 0;\n")
    h.write("    // Set or unset animation\n")
    h.write("    virtual void setAnimationEnabled(bool enabled);\n")
    h.write("   // Reload settings\n")
    if "settings" in actor:
        h.write("   virtual void reloadSettings(QSettings * settings) = 0;\n")
    else:
        h.write("   inline void reloadSettings(QSettings * ) {}\n")
    h.write("    // Change global state\n")
    h.write("    inline virtual void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current) { Q_UNUSED(old); Q_UNUSED(current); }\n")
    h.write("    // Actor methods\n")
    for method in actor["methods"]:
        try:
            ret = basetype_to_qtype(method["returnType"])
        except:
            ret = "void"
        name = "run"+name_to_cpp(method["name"], True)
        try:
            args = map(arg_to_qarg, method["arguments"])
        except:
            args = []
        h.write("    virtual "+ret+" "+name+"("+string.join(args, ", ")+") = 0;\n")
    h.write("public:\n")
    h.write("    // GUI access methods\n")
windows = dict()
if actor.has_key("gui") and actor["gui"].has_key("windows"):
    for window in actor["gui"]["windows"]:
        try:
            role = window["role"]
        except:
            if not windows.has_key("main"):
                role = "main"
            else:
                role = "secondary"+str(actor["gui"]["windows"].indexOf(window))
        windows[role] = window
if update:
    if windows.has_key("main"):
        h.write("    virtual QWidget* mainWidget() const = 0;\n")
    else:
        h.write("    inline QWidget* mainWidget() const { return 0; }\n")
    if windows.has_key("pult"):
        h.write("    virtual QWidget* pultWidget() const = 0;\n")
    else:
        h.write("    inline QWidget* pultWidget() const { return 0; }\n")


menus = dict()

if actor.has_key("gui") and actor["gui"].has_key("menus"):
    menus = actor["gui"]["menus"]

if update:
    if menus:
        h.write("    inline QList<QMenu*> moduleMenus() const { return QList<QMenu*>()");
        for menu in menus:
            name = "m_menu"+name_to_cpp(menu["title"], True)
            h.write(" << "+name)
        h.write("; }\n")
    else:
        h.write("    inline QList<QMenu*> moduleMenus() const { return QList<QMenu*>(); }\n")

    h.write("protected:\n")
    h.write("    // Useful methods\n")
    h.write("    void setError(const QString & errorText);\n")
    h.write("    QSettings* mySettings() const;\n")
    h.write("    // These methods works only for async calls,\n")
    h.write("    // calling them from not async does nothing\n")
    h.write("    void sleep(unsigned long secs);\n")
    h.write("    void msleep(unsigned long msecs);\n")
    h.write("    void usleep(unsigned long usecs);\n")
    h.write("    // Top-level menus\n")
    for menu in menus:
        h.write("    QMenu* m_menu"+name_to_cpp(menu["title"],True)+";\n")
    h.write("    // Actions\n")

def scan_actons(rootName, items):
    global h
    for item in items:
        name = rootName + name_to_cpp(item["title"],True)
        if item.has_key("items"):
            scan_actons(name, item["items"])
        else:
            h.write("    QAction* "+name+";\n")



if update:
    scan_actons("m_action", menus)

    h.write("\n}; // class %sBase\n" % actorCPPModuleName)
    h.write("\n} // namespace %s\n" % actorCPPNameSpace)
    h.write("#endif // "+hName.replace(".","_").upper()+"\n")
    h.close()

    cpp = open(cppName, "w")
    cpp.write(DO_NOT_EDIT)
    cpp.write("""
#include <QtCore>
#include <QtGui>
#include "extensionsystem/kplugin.h"
#include "%s"
#include "%s"

namespace %s {

%sBase::%sBase(ExtensionSystem::KPlugin * parent)
    : QObject(parent)
{
""" % (hName, name_to_cpp(actor["name"]).lower()+"plugin.h", actorCPPNameSpace, actorCPPModuleName, actorCPPModuleName))

if update:
    if menus:
        cpp.write("    static const QString currentLocaleName = QLocale().name();\n\n")

def set_item_title(varName, titleDict, setter):
    global cpp
    titles = list()
    for lang in titleDict.keys():
        if lang!="ascii":
            titles += [(lang, titleDict[lang])]
    fallbackTitle = titleDict["ascii"]
    if titles:
        for index, (lang, title) in enumerate(titles):
            if index==0:
                cpp.write("    if (currentLocaleName==\"%s\")\n"%lang)
            else:
                cpp.write("    else if (currentLocaleName==\"%s\") {\n"%lang)
            cpp.write(unicode("        %s->%s(QString::fromUtf8(\"%s\"));\n" % (varName, setter, title)).encode("utf-8"))
        cpp.write("    else\n")
        cpp.write("        %s->%s(\"%s\");\n" % (varName, setter, fallbackTitle))
    else:
        cpp.write("    %s->%s(\"%s\");\n" % (varName, setter, fallbackTitle))

if update:
    for menu in menus:
        menuVar = "m_menu"+name_to_cpp(menu["title"],True)
        cpp.write("    "+menuVar+" = new QMenu();\n")
        set_item_title(menuVar, menu["title"], "setTitle")
        cpp.write("\n")

def create_child_items(varName, rootName, items):
    for item in items:
        name = rootName + name_to_cpp(item["title"],True)
        if item.has_key("items"):
            # child menu
            cpp.write("    QMenu* %s = %s->addMenu(\"\");\n" % (name, varName))
            set_item_title(name, item["title"], "setTitle")
            create_child_items(name, name, item["items"])
        else:
            # action
            cpp.write("    m_action%s = %s->addAction(\"\");\n" % (name, varName))
            set_item_title("m_action"+name, item["title"], "setText")
        cpp.write("\n")

if update:
    for menu in menus:
        menuVar = "m_menu"+name_to_cpp(menu["title"],True)
        menuRoot = name_to_cpp(menu["title"],True)
        if menu.has_key("items"):
            create_child_items(menuVar, menuRoot, menu["items"])

    cpp.write("}\n\n")
    cpp.write("""
void $className::setError(const QString & errorText)
{
    $pluginClassName * plugin = qobject_cast<$pluginClassName*>(parent());
    plugin->s_errorText = errorText;
}

QSettings* $className::mySettings() const
{
    $pluginClassName * plugin = qobject_cast<$pluginClassName*>(parent());
    return plugin->mySettings();
}

void $className::sleep(unsigned long secs)
{
    $pluginClassName * plugin = qobject_cast<$pluginClassName*>(parent());
    plugin->sleep(secs);
}

void $className::msleep(unsigned long msecs)
{
    $pluginClassName * plugin = qobject_cast<$pluginClassName*>(parent());
    plugin->sleep(msecs);
}

void $className::usleep(unsigned long usecs)
{
    $pluginClassName * plugin = qobject_cast<$pluginClassName*>(parent());
    plugin->sleep(usecs);
}

void $className::setAnimationEnabled(bool enabled)
{
    Q_UNUSED(enabled);
}


""".\
    replace("$className",name_to_cpp(actor["name"],True)+"ModuleBase").\
    replace("$pluginClassName",name_to_cpp(actor["name"],True)+"Plugin")
    )
    cpp.write("} // namespace %s\n" % actorCPPNameSpace)
    cpp.close()

# --------------------------- plugin class

asyncClassDecl = ""
actorCPPNameSpace = "Actor" + name_to_cpp(actor["name"], firstUpper=True)
actorCPPName = name_to_cpp(actor["name"], firstUpper=True)

for m in actor["methods"]:
    if not m.has_key("async"):
        m["async"] = not m.has_key("returnType") or m["returnType"]=="void"

asyncClassDecl = "class "+actorCPPName+"AsyncRunThread* m_asyncRunThread;"

hName = actorCPPName.lower()+"plugin.h"
hIfnDef = hName.replace(".","_").upper()
cppName = actorCPPName.lower()+"plugin.cpp"
actorHasGui = "false"
mainIconName = ""
pultIconName = ""
if actor.has_key("gui"):
    actorHasGui = "true"
    if actor["gui"].has_key("windows"):
        for w in actor["gui"]["windows"]:
            try:
                if w["role"]=="main":
                    mainIconName = w["icon"]
            except:
                mainIconName = ""
            try:
                if w["role"]=="pult":
                    pultIconName = w["icon"]
            except:
                pultIconName = ""

if update:
    h = open(hName, "w")
    h.write(DO_NOT_EDIT)
    h.write("""
#ifndef $hFileIfdef
#define $hFileIfdef
#include <QtCore>
#include <QtGui>
#include "extensionsystem/kplugin.h"
#include "extensionsystem/declarativesettingspage.h"
#include "interfaces/actorinterface.h"

namespace $namespace {
    
class $pluginClassName
    : public ExtensionSystem::KPlugin
    , public Shared::ActorInterface
{
    friend class $baseClassName;
    friend class $asyncRunThreadName;
    Q_OBJECT
    Q_INTERFACES(Shared::ActorInterface)
public:
    // Constructor
    $pluginClassName();
    // Generic plugin information
    inline bool isGuiRequired() const { return $guiRequired; }
    // Actor information
    QStringList funcList() const;
    TypeList typeList() const;
    QString name() const;
    inline QString mainIconName() const { return "$mainIconName"; }
    inline QString pultIconName() const { return "$pultIconName"; }
    inline virtual bool handlesCustomTypeConstant() const { return $handlesCustomTypeConstant; }
    virtual QByteArray parseCustomTypeConstant(const QString & /*asciiTypeName*/,
                                               const QString & /*literal*/) const;
    // Actor GUI access
    QWidget* mainWidget();
    QWidget* pultWidget();
    QList<QMenu*> moduleMenus() const;
    inline QWidget * settingsEditorPage() { return m_settingsPage; }
    // Actor control
    void reset();
    void setAnimationEnabled(bool enabled);
    Shared::EvaluationStatus evaluate(quint32 index, const QVariantList &args);
    QVariant result() const;
    QVariantList algOptResults() const;
    QString errorText() const;
protected:
    QString initialize(const QStringList&);
    void sleep(unsigned long secs);
    void msleep(unsigned long msecs);
    void usleep(unsigned long usecs);
    void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
private slots:
    void handleSettingsChanged();
private:
    template <typename T> inline static QVector<T> toVector1(const QVariant & v)
    {
        const QVariantList l = v.toList();
        QVector<T> result;
        result.resize(l.size());
        for (int i=0; i<l.size(); i++) {
            result[i] = qvariant_cast<T>(l[i]);
        }
        return result;
    }
    template <typename T> inline static QVector< QVector<T> > toVector2(const QVariant & v)
    {
        const QVariantList l = v.toList();
        QVector< QVector<T> > result;
        result.resize(l.size());
        for (int i=0; i<l.size(); i++) {
            const QVariantList ll = l[i].toList();
            result[i].resize(ll.size());
            for (int j=0; j<ll.size(); j++) {
                result[i][j] = qvariant_cast<T>(ll[j]);
            }
        }
        return result;
    }
    template <typename T> inline static QVector< QVector< QVector<T> > > toVector3(const QVariant & v)
    {
        const QVariantList l = v.toList();
        QVector< QVector< QVector<T> > > result;
        result.resize(l.size());
        for (int i=0; i<l.size(); i++) {
            const QVariantList ll = l[i].toList();
            result[i].resize(ll.size());
            for (int j=0; j<ll.size(); j++) {
                const QVariantList lll = ll[j].toList();
                result[i][j].resize(lll.size());
                for (int k=0; k<lll.size(); k++) {
                    result[i][j][k] = qvariant_cast<T>(lll[k]);
                }
            }
        }
        return result;
    }
    void updateSettings();
    class $baseClassName* m_module;
    $asyncClassDecl
    QString s_errorText;
    QVariant v_result;
    QVariantList l_optResults;
    ExtensionSystem::DeclarativeSettingsPage * m_settingsPage;
signals:
    void sync();
};

} // $namespace

#endif // $hFileIfdef
"""
    .replace("$hFileIfdef", hIfnDef)
    .replace("$namespace", actorCPPNameSpace)
    .replace("$pluginClassName", actorCPPName+"Plugin")
    .replace("$baseClassName", actorCPPName+"ModuleBase")
    .replace("$asyncRunThreadName", actorCPPName+"AsyncRunThread")
    .replace("$guiRequired", actorHasGui)
    .replace("$mainIconName", mainIconName)
    .replace("$pultIconName", pultIconName)
    .replace("$asyncClassDecl", asyncClassDecl)
    .replace("$handlesCustomTypeConstant", handlesCustomTypeConstant)
    .encode("utf8")
    )

    h.close()
    
def kumir_custom_typename(name, locale="ascii"):
    # TODO implement non-russian language
    if not "types" in actor.keys():
        return u""
    for tp in actor["types"]:
        nm = tp["name"]
        if type(nm)==dict:
            ascii = nm["ascii"]
            ru = nm["ru_RU"]
        else:
            ascii = nm
            ru = nm
        if ascii==name:
            return ru
    return u""

def kumir_signature(method, locale="ascii"):
    # TODO implement non-russian language
    res = u"алг "
    if method.has_key("returnType") and method["returnType"]!="void":
        if method["returnType"]=="int": res += u"цел "
        elif method["returnType"]=="double": res += u"вещ "
        elif method["returnType"]=="bool": res += u"лог "
        elif method["returnType"]=="char": res += u"сим "
        elif method["returnType"]=="string": res += u"лит "
        else: res += kumir_custom_typename(method["returnType"])+" "
    if type(method["name"])==dict:
        if "ru_RU" in method["name"]:
            res += method["name"]["ru_RU"]
        else:
            res += method["name"]["ascii"]
    else:
        assert type(method["name"])==str or type(method["name"])==unicode
        res += method["name"]
    if method.has_key("arguments"):
        args = list()
        for arg in method["arguments"]:
            if arg["baseType"]=="int": baseType=u"цел"
            elif arg["baseType"]=="double": baseType=u"вещ"
            elif arg["baseType"]=="bool": baseType=u"лог"
            elif arg["baseType"]=="char": baseType=u"сим"
            elif arg["baseType"]=="string": baseType=u"лит"
            else: baseType = kumir_custom_typename(arg["baseType"])
            try:
                dim = int(arg["dim"])
            except:
                dim = 0
            if dim: baseType += u"таб"
            acc = ""
            try:
                if arg["access"]=="out": acc=u"рез "
                if arg["access"]=="in/out": acc=u"аргрез "
            except:
                pass
            assert "name" in arg
            assert type(arg["name"])==unicode or type(arg["name"])==str or type(arg["name"])==dict
            if type(arg["name"])==unicode or type(arg["name"])==str:
                argname = arg["name"]
            elif type(arg["name"])==dict:
                if "ru_RU" in arg["name"]:
                    argname = arg["name"]["ru_RU"]
                else:
                    argname = arg["name"]["ascii"]
            a = acc+baseType+" "+argname
            if dim:
                dummyBounds = ["0:0"] * dim
                a += "["+string.join(dummyBounds, ",")+"]"
            args += [a]
        res += "("+string.join(args, ", ")+")"
    return res

makeMethods = reduce(lambda x,y: x+"\n    "+y,
    map(lambda x: "result << QString::fromUtf8(\""+kumir_signature(x)+"\");", actor["methods"])
    )

def make_method_call(method, fromThread=False):
    if method["async"] and not fromThread:
        return """ {
            m_asyncRunThread->init(index, args);
            m_asyncRunThread->start();
            return Shared::ES_Async;
        }
    """
    else:
        call = "m_module->run"+name_to_cpp(method["name"], True)+"("
        arglines = []
        optresults = ""
        if method.has_key("arguments"):
            for index, arg in enumerate(method["arguments"]):
                try:
                    dim = int(arg["dim"])
                except:
                    dim = 0
                qtype = basetype_to_qtype(arg["baseType"])
                for i in range(0,dim): qtype = vector_type(qtype)
                argline = qtype + " x"+str(index+1);
                if not arg.has_key("access") or "in" in arg["access"]:
                    argline += " = "
                    if fromThread: argline += "l_"
                    argvar = "args[%i]" % index
                    bt = basetype_to_qtype(arg["baseType"])
                    if dim:
                        argline += "toVector%i<%s>(%s)" % (dim, bt, argvar)
                    elif qtype in ["int", "qreal", "bool", "QString", "QChar"] or dim>0:
                        argline += "qvariant_cast<%s>(%s)" % (bt, argvar)
                    else:
                        argline += "customTypeFromQVariant<%s>(%s)" % (bt, argvar)
                    
                if arg.has_key("access") and "out" in arg["access"]:
                    optresults += "        l_optResults << x"+str(index+1)+";\n"
                argline += ";"
                arglines += [argline]
                if index: call += ", "
                call += "x"+str(index+1)
        call += ")"
        if optresults:
            optresults = "        l_optResults.clear();\n"+optresults
        if method.has_key("returnType") and method["returnType"]!="void":
            if method["returnType"] in ["int", "double", "string", "bool", "char"]:
                call = "v_result = QVariant::fromValue(%s)" % call
            else:
                call = "v_result = customTypeToQVariant<%s>(%s)" % (basetype_to_qtype(method["returnType"]), call)
            if fromThread:
                call = "m_plugin->"+call
            if optresults:
                ret = "Shared::ES_StackRezResult"
            else:
                ret = "Shared::ES_StackResult"
        else:
            if optresults:
                ret = "Shared::ES_RezResult"
            else:
                ret = "Shared::ES_NoResult"
        if not fromThread:
            return """ {
            $vardecl
            $call
            $optresults
            if (s_errorText.length()>0)
                return Shared::ES_Error;
            return $ret;
        }
        """. \
        replace("$vardecl", string.join(arglines, "\n        ")). \
        replace("$call", call+";"). \
        replace("$ret", ret). \
        replace("$optresults", optresults)
        else:
            return """ {
            $vardecl
            $call
            break;
            }
        """. \
        replace("$vardecl", string.join(arglines, "\n        ")). \
        replace("$call", call+";"). \
        replace("$ret", ret)

evaluateMethods = map(make_method_call, actor["methods"])
evaluateMethods = map(lambda x: ("        case %#06x:"%x[0])+x[1], enumerate(evaluateMethods))
evaluateMethods = """switch (index) {
%s
        default: {
            s_errorText = "Unknown method index";
            return Shared::ES_Error;
        }
    }""" % string.join(evaluateMethods, "\n")

evaluateMethodsThr = "switch(i_index) { \n"
for index, method in enumerate(actor["methods"]):
    if method["async"]:
        case = make_method_call(method, True)
        evaluateMethodsThr += "        case %#06x: %s" % (index, case)
evaluateMethodsThr += """
        default: {
            m_plugin->s_errorText = "Unknown method index";
        }
    }"""

settingsPageCreation = ""
if "settings" in actor:
    settingsPageCreation  = "    QMap<QString,ExtensionSystem::DeclarativeSettingsPage::Entry> entries;\n"
    settingsPageCreation += "    ExtensionSystem::DeclarativeSettingsPage::Entry entry;\n"
    for key, value in actor["settings"].items():
        t = value["type"]
        title = value["title"]["ru_RU"] # TODO non-russuan lanugage support
        deflt = value["default"]
        if t=="int": tp="Integer"
        if t=="double": tp="Double"
        if t=="string": tp="String"
        if t=="char": tp="Char"
        if t=="bool": tp="Bool"
        if t=="color": tp="Color"
        if t=="font": tp="Font"
        if t in ["string", "color", "font"]:
            deflt = 'QString::fromUtf8("'+deflt+'")'
        if t=="char":
            deflt = 'QString::fromUtf8("'+deflt+'")[0]'
        deflt = unicode(deflt)
        settingsPageCreation += "    entry.title = QString::fromUtf8(\"%s\");\n" % title
        settingsPageCreation += "    entry.type = ExtensionSystem::DeclarativeSettingsPage::%s;\n" % tp
        settingsPageCreation += "    entry.defaultValue = %s;\n" % deflt
        if "minimum" in value:
            settingsPageCreation += "    entry.minimumValue = %s;\n" % unicode(value["minimum"])
        else:
            settingsPageCreation += "    entry.minimumValue = QVariant::Invalid;\n"
        if "maximum" in value:
            settingsPageCreation += "    entry.maximumValue = %s;\n" % unicode(value["maximum"])
        else:
            settingsPageCreation += "    entry.maximumValue = QVariant::Invalid;\n"
        settingsPageCreation += "    entries[\"%s\"] = entry;\n" % key
    settingsPageCreation += """
    m_settingsPage = new ExtensionSystem::DeclarativeSettingsPage(\n
                                          \"$pluginName\",
                                          QString::fromUtf8("$actorLocalizedName"),
                                          mySettings(),
                                          entries
                                        );
    connect(m_settingsPage, SIGNAL(settingsChanged()), this, SLOT(handleSettingsChanged()));
    """\
    .replace("$pluginName", actorCPPName+"Plugin") \
    .replace("$actorLocalizedName", actor["name"]["ru_RU"])

makeTypes = ""
if "types" in actor.keys():
    makeTypes += "Shared::ActorInterface::CustomType tp;\n"
    for tp in actor["types"]:
        name = tp["name"]
        if type(name)==dict:
            name = name["ascii"]
        typename = tp["cppdecl"]
        if typename.startswith("struct "):
            p = typename.index("{")
            typename = typename[7:p]
        if typename.startswith("class "):
            p = typename.index("{")
            typename = typename[6:p]
        kum_name = kumir_custom_typename(name)
        makeTypes += "    tp = Shared::ActorInterface::CustomType(QString::fromUtf8(\""+kum_name+"\"), sizeof("+typename+"));\n"
        makeTypes += "    result << tp;\n"



if update:
    cpp = open(cppName, "w")
    cpp.write(DO_NOT_EDIT)
    cpp.write(u"""
#include "$includePlugin"
#include "$includeModule"

namespace $namespace {

class $threadClassName
    : public QThread
{
public:
    inline $threadClassName($className * plugin, $moduleClassNameBase * module)
        : QThread(plugin)
    {
        m_plugin = plugin;
        m_module = module;
    }
    inline void init(quint32 index, const QVariantList & args)
    {
        i_index = index;
        l_args = args;
    }
    inline void asleep(unsigned long secs) { sleep(secs); }
    inline void amsleep(unsigned long msecs) { sleep(msecs); }
    inline void ausleep(unsigned long usecs) { sleep(usecs); }
private:
    void run();
    quint32 i_index;
    QVariantList l_args;
    $className * m_plugin;
    $moduleClassNameBase * m_module;
};

void $threadClassName::run()
{
    $threadRunBody
}

$className::$className()
    : ExtensionSystem::KPlugin()
{
    m_module = 0;
    m_asyncRunThread = 0;
    m_settingsPage = 0;
}

void $className::changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current)
{
    m_module->changeGlobalState(old, current);
}

QString $className::initialize(const QStringList&)
{
    m_module = new $moduleClassName(this);
    m_asyncRunThread = new $threadClassName(this, m_module);
    connect(m_asyncRunThread,
        SIGNAL(finished()),
        this,
        SIGNAL(sync()) );
    
$settingsPageCreation
    return QString::fromAscii("");
}

QStringList $className::funcList() const
{
    QStringList result;
    $makeMethods
    return result;
}

Shared::ActorInterface::TypeList $className::typeList() const
{
    Shared::ActorInterface::TypeList result;
    $makeTypes
    return result;
}

QByteArray $className::parseCustomTypeConstant(const QString & typeName, const QString & literal) const
{
    QByteArray result;
    $parseCustomTypeConstantBody
    return result;
}

QString $className::name() const
{
    return QString::fromUtf8(\"$actorName\");
}

QWidget* $className::mainWidget() 
{
    return m_module->mainWidget();
}

QWidget* $className::pultWidget()
{
    return m_module->pultWidget();
}

QList<QMenu*> $className::moduleMenus() const
{
    return m_module->moduleMenus();
}

void $className::reset()
{
    m_module->reset();
}

void $className::updateSettings()
{
    if (m_settingsPage) {
        m_settingsPage->setSettingsObject(mySettings());
    }
    if (m_module)
        m_module->reloadSettings(mySettings());
}

void $className::handleSettingsChanged()
{
    if (m_module)
        m_module->reloadSettings(mySettings());
}

void $className::setAnimationEnabled(bool enabled)
{
    if (m_module)
        m_module->setAnimationEnabled(enabled);
}

template <typename T>
T customTypeFromQVariant(const QVariant & value)
{
    T res;
    QByteArray valData = value.toByteArray();
    memcpy(&res, valData.constData(), sizeof(T));
    return res;
}

template <typename T>
QVariant customTypeToQVariant(const T & value)
{
    char data[sizeof(T)];
    memcpy(&data, &value, sizeof(T));
    QByteArray valData(data);
    return QVariant(valData);
}

Shared::EvaluationStatus $className::evaluate(quint32 index, const QVariantList & args)
{
    s_errorText.clear();
    v_result = QVariant::Invalid;
    l_optResults.clear();
    $evaluateMethods
}

void $className::sleep(unsigned long secs)
{
    if (QThread::currentThread()==m_asyncRunThread) {
        m_asyncRunThread->asleep(secs);
    }
}

void $className::msleep(unsigned long msecs)
{
    if (QThread::currentThread()==m_asyncRunThread) {
        m_asyncRunThread->amsleep(msecs);
    }
}

void $className::usleep(unsigned long usecs)
{
    if (QThread::currentThread()==m_asyncRunThread) {
        m_asyncRunThread->ausleep(usecs);
    }
}

QVariant $className::result() const
{
    return v_result;
}

QVariantList $className::algOptResults() const
{
    return l_optResults;
}

QString $className::errorText() const
{
    return s_errorText;
}

}

Q_EXPORT_PLUGIN($namespace::$className)

"""
    .replace("$includePlugin", hName)
    .replace("$includeModule", actorCPPName.lower()+"module.h")
    .replace("$namespace", actorCPPNameSpace)
    .replace("$className", actorCPPName+"Plugin")
    .replace("$moduleClassName", actorCPPName+"Module")
    .replace("$threadClassName", actorCPPName+"AsyncRunThread")
    .replace("$makeMethods", makeMethods)
    .replace("$evaluateMethods", evaluateMethods)
    .replace("$threadClassName", actorCPPName+"AsyncRunThread")
    .replace("$threadRunBody", evaluateMethodsThr)
    .replace("$actorName", actor["name"]["ru_RU"])
    .replace("$settingsPageCreation", settingsPageCreation)
    .replace("$makeTypes", makeTypes)
    .replace("$parseCustomTypeConstantBody", parseCustomTypeConstantBody)
    .encode("utf-8")
    )
    cpp.close()

# -------------------- plugin spec file
if update:
    pluginName = "Actor"+name_to_cpp(actor["name"],True)
    spec = open(pluginName+".pluginspec", "w")
    spec.write("name    = %s\ngui     = %s" % (pluginName, actorHasGui))
    spec.close()


# -------------------- generate skeletons

if not "--project" in sys.argv:
    sys.exit(0)

guiIfNeed = ""
if actor.has_key("gui"):
    guiIfNeed = "public:\n    // GUI access methods\n"
    if windows.has_key("main"):
        guiIfNeed += "    QWidget* mainWidget() const;\n"
    if windows.has_key("pult"):
        guiIfNeed += "    QWidget* pultWidget() const;\n"

actorMethods = []
for method in actor["methods"]:
    try:
        ret = basetype_to_qtype(method["returnType"])
    except:
        ret = "void"
    name = "run"+name_to_cpp(method["name"], True)
    try:
        args = map(arg_to_qarg, method["arguments"])
    except:
        args = []
    actorMethods += [(ret, name+"("+string.join(args,", ")+")")]

actorMethodsForHeader = map(lambda x: "    "+x[0]+" "+x[1]+";\n", actorMethods)
actorMethodsForHeader = reduce(lambda x,y: x+y, actorMethodsForHeader, "")

className = name_to_cpp(actor["name"], True)+"Module"

settingsIfNeed = ""

if "settings" in actor:
    settingsIfNeed = "    void reloadSettings(QSettings * settings);\n"

h = open(className.lower()+".h", "w")
h.write("""/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents. 
You should change it corresponding to functionality.
*/

#ifndef $ifndef
#define $ifndef

#include <QtCore>
#include <QtGui>
#include "extensionsystem/kplugin.h"
#include "$baseClassHeader"

namespace $namespace {

class $className
    : public $baseClassName
{
    Q_OBJECT
public:
    // Constructor

    $className(ExtensionSystem::KPlugin * parent);
$constantParser
public slots:
    // Reset actor state before program starts
    void reset();
    // Set animation flag
    void setAnimationEnabled(bool enabled);
    // Actor methods
$methods
$guiIfNeed
$settingsIfNeed
}; // $className
    
} // $namespace

#endif // $ifndef
"""
.replace("$ifndef", className.upper()+"_H")
.replace("$baseClassHeader", className.lower()+"base.h")
.replace("$className", className)
.replace("$baseClassName", className+"Base")
.replace("$namespace", actorCPPNameSpace)
.replace("$guiIfNeed", guiIfNeed)
.replace("$methods", actorMethodsForHeader)
.replace("$settingsIfNeed", settingsIfNeed)
.replace("$constantParser", parseCustomConstantH)
)
h.close()

cpp = open(className.lower()+".cpp", "w")
cpp.write("""/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents. 
You should change it corresponding to functionality.
*/

#include <QtCore>
#include <QtGui>
#include "$classHeader"

namespace $namespace {

$className::$className(ExtensionSystem::KPlugin * parent)
    : $classNameBase(parent)
{
    /* TODO 
    implement class Constructor
    */
}

void $className::reset()
{
    /* TODO
    This method is called when actor resets its state before program starts.
    */
}

void $className::setAnimationEnabled(bool enabled)
{
    /* TODO
    Set internal flag for animation
    */
}

$constantsParsers

"""
.replace("$classHeader", className.lower()+".h")
.replace("$namespace", actorCPPNameSpace)
.replace("$className", className)
.replace("$constantsParsers", parseCustomConstantCPP)
)

if windows.has_key("main"):
    cpp.write("""
QWidget* $className::mainWidget() const
{
    /* TODO
    This method should return a pointer to main widget.
    NOTE: This metod should NOT create main widget -- just return!
    */
    return 0;
}
""".replace("$className", className))

if windows.has_key("pult"):
    cpp.write("""
QWidget* $className::pultWidget() const
{
    /* TODO
    This method should return a pointer to pult widget.
    NOTE: This metod should NOT create pult widget -- just return!
    */
    return 0;
}
""".replace("$className", className))

for ret, signature in actorMethods:
    if ret=="QString":
        default = " \"\"";
    elif ret=="QChar":
        default = " ' '"
    elif ret=="int":
        default = " 0";
    elif ret=="qreal":
        default = " 0.0"
    elif ret=="bool":
        default = " false";
    else:
        default = " "+ret+"()"
    cpp.write("""
$returnType $className::$signature
{
    /* TODO implement me */
    return$default;
}

"""
.replace("$className", className)
.replace("$returnType", ret)
.replace("$default", default)
.replace("$signature", signature)
)
    
if "settings" in actor:
    cpp.write("""

void $className::reloadSettings(QSettings * settings)
{
    // TODO handle settings changed
}

""".replace("$className", className))

cpp.write("""    
} // $namespace
"""
)

cpp.close()

#------------------  CMake project file
cmakelists = open("CMakeLists.txt", "w")
cmakelists.write("""
include(../../kumir2_plugin.cmake)

set(SOURCES
    $actorNamemodule.cpp
)

set(MOC_HEADERS
    $actorNamemodule.h
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.cpp ${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.h ${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.cpp ${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.h ${CMAKE_CURRENT_BINARY_DIR}/$pluginName.pluginspec
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/gen_actor_source.py --update ${CMAKE_CURRENT_SOURCE_DIR}/$jsonName
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/$jsonName ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripts/gen_actor_source.py
)

add_custom_target($pluginNamePluginSpec ALL ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/$pluginName.pluginspec ${PLUGIN_OUTPUT_PATH}
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/$pluginName.pluginspec
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.moc.cpp
    COMMAND ${QT_MOC_EXECUTABLE} -I${CMAKE_SOURCE_DIR}/src/shared -o${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/$actorNamemodulebase.h
)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.moc.cpp
    COMMAND ${QT_MOC_EXECUTABLE} -I${CMAKE_SOURCE_DIR}/src/shared -o${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.moc.cpp ${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.h
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/$actorNameplugin.h
)

set(SOURCES2
    $actorNamemodulebase.cpp
    $actorNameplugin.cpp
)

set(MOC_SOURCES2
    $actorNamemodulebase.moc.cpp
    $actorNameplugin.moc.cpp
)

qt4_wrap_cpp(MOC_SOURCES ${MOC_HEADERS})

install(FILES ${PLUGIN_OUTPUT_PATH}/$pluginName.pluginspec DESTINATION ${PLUGINS_DIR})
handleTranslation($pluginName)
add_library($pluginName SHARED ${MOC_SOURCES} ${SOURCES} ${MOC_SOURCES2} ${SOURCES2})
target_link_libraries($pluginName ${QT_LIBRARIES} ExtensionSystem)

install(TARGETS $pluginName DESTINATION ${PLUGINS_DIR})

"""
.replace("$actorName", actorCPPName.lower())
.replace("$pluginName", "Actor"+actorCPPName)
.replace("$jsonName", sys.argv[-1])
)
cmakelists.close()
