/**
 * UGENE - Integrated Bioinformatics Tools.
 * Copyright (C) 2008-2013 UniPro <ugene@unipro.ru>
 * http://ugene.unipro.ru
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

 /** Creates the ParametersWidget layout and the first active tab (without parameters). */

function lwInitConteiner(container, activeTabId) {
    var mainHtml =
        '<div class="tree" id="treeRoot">' +
           '<ul>'  +
           '</ul>' +
        '</div>';

    container.innerHTML = mainHtml;
    initializeCopyMenu();
}

function addChildrenElement(parentObject, elemTag, elemHTML) {
    var newElem = document.createElement(elemTag);
    newElem.innerHTML = elemHTML;
    parentObject.appendChild(newElem);
    return newElem;
}
function addChildrenNode(parentNode, nodeContent, spanId, nodeClass) {

    var newListElem = addChildrenElement(parentNode, 'LI', '');
    var span = addChildrenElement(newListElem, 'span', nodeContent);
    span.setAttribute('title', 'Collapse this branch');

    span.setAttribute('onclick', 'collapseNode(this)');
    span.setAttribute('onmouseup', 'return contextmenu(event, this);');

    span.id = spanId;
    span.className = nodeClass;
    var newList = addChildrenElement(newListElem, 'UL', '');
    return newList;
}

function collapseNode(element) {
        var children = $(element).parent('li.parent_li').find(' > ul > li');
        if (children.is(":visible")) {
            children.hide('fast');
            $(element).attr('title', 'Expand this branch').find(' > i').addClass('icon-plus-sign').removeClass('icon-minus-sign');
        } else {
            children.show('fast');
            $(element).attr('title', 'Collapse this branch').find(' > i').addClass('icon-minus-sign').removeClass('icon-plus-sign');
        }
};

function lwAddTreeNode(nodeName, activeTabName, activeTabId, content, contentType) {
    var actorTab = document.getElementById(activeTabName);
    if(actorTab === null) {
        var root = document.getElementById("treeRoot");
        var rootList = root.getElementsByTagName('ul')[0];
        actorTab = addChildrenNode(rootList, '<i class="icon-minus-sign"></i>' + activeTabName, activeTabName + '_span', 'badge tool-node');
        actorTab.id = activeTabName;
    }

    var launchNodeId = activeTabName + nodeName + "_l";
    var launchNode = document.getElementById(launchNodeId);
    if(null === launchNode) {
        launchNode = addChildrenNode(actorTab, nodeName, launchNodeId + '_span', 'badge badge-success');
        launchNode.id = launchNodeId;
    }

    var idBase = activeTabId + nodeName;
    if(content) {
        content = content.replace(/break_line/g, '<br>');
        content = content.replace(/(<br>){3,}/g, '<br><br>');
        content = content.replace(/s_quote/g, '\'');
        content = content.replace(/b_slash/g, '\\');
    }
    else {
        enabledCollapsing();
        return;
    }
    var infoNode;
    infoNode = document.getElementById(launchNodeId + '_info');
    switch(contentType) {
        case "error":
                addContent(launchNode, 'Error log', idBase + '_er', 'badge badge-important', content);
                var launchSpan = document.getElementById(launchNodeId + '_span');
                launchSpan.className = 'badge badge-important';
            break;
        case "output":
            addContent(launchNode, 'Output log', idBase + '_out', 'badge badge-info', content);
            break;
        case "tool_name":
             if(null === infoNode) {
                infoNode = addChildrenNode(launchNode, 'Run info', idBase + '_id1', 'badge run-info');
                infoNode.id = launchNodeId + '_info';
             }
             addContent(infoNode, 'Tool name', idBase + '_tool', 'badge tool-info', content);
             break;
        case "program":
            if(null === infoNode) {
                infoNode = addChildrenNode(launchNode, 'Run info', idBase + '_id2', 'badge run-info');
                infoNode.id = launchNodeId + '_info';
            }
            addContent(infoNode, 'Executable file', idBase + '_program', 'badge program-path', content);
            break;
        case "arguments":
            if(null === infoNode) {
                infoNode = addChildrenNode(launchNode, 'Launch info', idBase + '_id3', 'badge run-info');
                infoNode.id = launchNodeId + '_info';
            }
            addContent(infoNode, 'Arguments', idBase + '_args', 'badge tool-args', content);
            break;
    }
    enabledCollapsing();
}
function addContent(parentNode, contentHead, nodeId, contentType, content) {
    var node = document.getElementById(nodeId);

    if(node !== null) {
        node.innerHTML += content;
    } else if(content) {
        node = addChildrenNode(parentNode, contentHead, nodeId + '_label', contentType);
        content = content.replace(/^(<br>)+/, "");
        addChildrenNode(node, content, nodeId, 'content');
    }
}

function enabledCollapsing() { 
    $('.tree li:has(ul)').addClass('parent_li').find(' > span').attr('title', 'Collapse this branch');
}
