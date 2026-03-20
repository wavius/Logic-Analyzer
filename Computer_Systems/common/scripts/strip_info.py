import xml.etree.ElementTree
import sys

if len(sys.argv) != 2:
    print ("Usage: " + sys.argv[0] + " <sopcinfo file name>")
    exit()

def printchildren( node ):
    for child in reversed(node):
        print(child.tag, child.attrib)

def parseparameterforvalue( node ):
    for child in reversed(node):
        if child.tag == "value":
            continue
        node.remove(child)

def parseinterface( node ):
    changed_attr = True;
    while changed_attr:
        changed_attr = False;
        for key in node.attrib:
            if key == "kind":
                continue
            if key == "name":
                continue
            changed_attr = True;
            del node.attrib[key]
            break
    for child in reversed(node):
        if child.tag == "memoryBlock":
            continue
        if child.tag == "assignment":
            continue
        node.remove(child)

def parsemodule( node ):
    changed_attr = True;
    while changed_attr:
        changed_attr = False;
        for key in node.attrib:
            if key == "kind":
                continue
            if key == "name":
                continue
            if key == "path":
                continue
            changed_attr = True;
            del node.attrib[key]
            break
    for child in reversed(node):
        if child.tag == "interface":
            if child.attrib['kind'] == "avalon_master":
                parseinterface( child )
                continue
            if child.attrib['kind'] == "altera_axi_master":
                parseinterface( child )
                continue
            if child.attrib['kind'] == "avalon_slave":
                parseinterface( child )
                continue
            if child.attrib['kind'] == "altera_axi_slave":
                parseinterface( child )
                continue
        if child.tag == "parameter":
            parseparameterforvalue( child )
            if child.attrib['name'] == "resetAbsoluteAddr":
                continue
            if child.attrib['name'] == "exceptionAbsoluteAddr":
                continue
            if child.attrib['name'] == "muldiv_divider":
                continue
            if child.attrib['name'] == "muldiv_multiplierType":
                continue
            if child.attrib['name'] == "debug_jtagInstanceID":
                continue
            if child.attrib['name'] == "autoInitializationFileName":
                continue
            if child.attrib['name'] == "initializationFileName":
                continue
            if child.attrib['name'] == "initMemContent":
                continue
            if child.attrib['name'] == "useNonDefaultInitFile":
                continue
            if child.attrib['name'] == "id":
                continue
            if child.attrib['name'] == "timestamp":
                continue
        node.remove(child)

def removeallchildren( node ):
    for child in reversed(node):
        node.remove(child)

filename = sys.argv[1]
tree = xml.etree.ElementTree.parse(filename)
root = tree.getroot()

for child in reversed(root):
    if child.tag == "connection":
        removeallchildren( child )
        continue
    if child.tag == "module":
        parsemodule( child )
        continue

    root.remove(child)


tree.write(filename + ".amp")

#for atype in root.findall('module'):
#    print(atype.get('name'))
    
#print("This line will be printed.")
