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
    newListElem.className = 'parent_li';
    var span = addChildrenElement(newListElem, 'span', nodeContent);
    span.setAttribute('title', 'Collapse this branch');

    span.setAttribute('onclick', 'collapseNode(this)');
    span.setAttribute('onmouseover', 'highlightElement(this, event, true)');
    span.setAttribute('onmouseout', 'highlightElement(this, event, false)');
    span.setAttribute('onmouseup', 'return contextmenu(event, this);');

    span.id = spanId;
    span.className = nodeClass;
    var newList = addChildrenElement(newListElem, 'UL', '');

    return newList;
}

function collapseNode(element) {
        var children = $(element).parent('li.parent_li').find(' > ul > li');
        if (children.is(":visible") == $(element).is(":visible")) {
            children.hide(0);
            $(element).attr('title', 'Expand this branch');
        } else {
            children.show(0);
            $(element).attr('title', 'Collapse this branch');
        }
};

function lwAddTreeNode(nodeName, activeTabName, activeTabId, content, contentType) {
    var actorTab = document.getElementById(activeTabName);
    if(actorTab === null) {
        var root = document.getElementById("treeRoot");
        var rootList = root.getElementsByTagName('ul')[0];
        actorTab = addChildrenNode(rootList, activeTabName, activeTabName + '_span', 'badge tool-node');
        actorTab.id = activeTabName;
        var activeTabSpan = document.getElementById(activeTabName + '_span');
    }

    var launchNodeId = activeTabName + nodeName + "_l";
    var launchNode = document.getElementById(launchNodeId);
    var idBase = activeTabId + nodeName;
    var isLaunchNodeCreated = false;
    if(null === launchNode) {
        isLaunchNodeCreated = true;
        var activeTabSpan = document.getElementById(activeTabName + '_span');
        launchNode = addChildrenNode(actorTab, nodeName, launchNodeId + '_span', 'badge badge-success');
        launchNode.id = launchNodeId;
        var launchSpan = document.getElementById(launchNodeId + '_span');

        var copyRunInfoButton = document.createElement('button');
        copyRunInfoButton.className = "copyRunInfo";
        copyRunInfoButton.setAttribute("title", "Copy external tool run string");
        copyRunInfoButton.setAttribute("onclick", "copyRunInfo(event, \'" + idBase + "\'); return false;");

        copyRunInfoButton.setAttribute('onmousedown', 'return onButtonPressed(this, event);');
        copyRunInfoButton.setAttribute('onmouseup',   'return onButtonReleased(this, event);');

        copyRunInfoButton.setAttribute('onmouseover', 'highlightElement(this, event, true)');
        copyRunInfoButton.setAttribute('onmouseout', 'highlightElement(this, event, false)');

        launchSpan.appendChild(copyRunInfoButton);
        if(activeTabSpan.getAttribute('title') === 'Expand this branch') {
            collapseNode(activeTabSpan);
        }
    }
    if(content) {
        content = content.replace(/break_line/g, '<br>');
        content = content.replace(/(<br>){3,}/g, '<br><br>');
        content = content.replace(/s_quote/g, '\'');
        content = content.replace(/b_slash/g, '\\');
    }
    else {
        return;
    }

    var infoNode;
    infoNode = document.getElementById(launchNodeId + '_info');
    var launchSpan = document.getElementById(launchNodeId + '_span');
    switch(contentType) {
        case "error":
                addContent(launchNode, 'Error log', idBase + '_er', 'badge badge-important', content);
                launchSpan.className = 'badge badge-important';
            break;
        case "output":
            addContent(launchNode, 'Output log', idBase + '_out', 'badge badge-info', content);
            break;
        case "program":
            if(null === infoNode) {
                infoNode = addInfoNode(launchNode);
            }
            addContent(infoNode, 'Executable file', idBase + '_program', 'badge program-path', content);
            break;
        case "arguments":
            if(null === infoNode) {
                infoNode = addInfoNode(launchNode);
            }
            addContent(infoNode, 'Arguments', idBase + '_args', 'badge tool-args', content);
            break;
    }
    if(isLaunchNodeCreated && launchSpan.getAttribute('title') !== 'Expand this branch'){
        launchSpan = document.getElementById(launchNodeId + '_span');
        collapseNode(launchSpan);
    }
}
function onButtonPressed(element, event) {
    if(1 === event.which) {
        $(element).addClass('pressed');
    }
    event.stopPropagation();
    return false;
}
function onButtonReleased(element, event) {
    $(element).removeClass('pressed');
    event.stopPropagation();
    return false;
}

function addInfoNode(launchNode) {
    if(null === launchNode) {
        return null;
    }
    var launchNodeId = launchNode.id;
    infoNode = addChildrenNode(launchNode, 'Run info', launchNodeId + '_info_span', 'badge run-info');
    infoNode.id = launchNodeId + '_info';

    var launchSpan = document.getElementById(launchNodeId + '_span');
    if(null === launchSpan) {
        return infoNode;
    }
    if(launchSpan.getAttribute('title') === 'Expand this branch') {
        collapseNode(launchSpan);
    }
    return infoNode;
}

function addContent(parentNode, contentHead, nodeId, contentType, content) {
    var node = document.getElementById(nodeId);

    if(node !== null) {
        node.innerHTML += content;
    } else if(content) {
        node = addChildrenNode(parentNode, contentHead, nodeId + '_label', contentType);
        content = content.replace(/^(<br>)+/, "");
        addChildrenNode(node, content, nodeId, 'content');
        var parentSpan = document.getElementById(parentNode.id + '_span');
        if(null === parentSpan) {
            return;
        }
        if(parentSpan.getAttribute('title') === 'Expand this branch') {
            collapseNode(parentSpan);
        }
    }
}

function copyRunInfo(event, idBase) {
    var resultString = "";

    resultString += '\"' + getElementText(idBase + '_program') + "\" ";
    resultString += getElementText(idBase + '_args');
    agent.setClipboardText(resultString);
    event.stopPropagation();
}
//Get text of the element without linebreak symbols
function getElementText(elementId) {
    var pathNode = document.getElementById(elementId);
    var resultString = "";
    if(pathNode !== null) {
        resultString = pathNode.innerHTML;
        resultString = resultString.replace(/<br>/g, ' ');
    }
    return resultString;
}

function highlightElement(element, e, isHighlighted)  {
    if(true === isHighlighted) {
        $('li span').removeClass('hoverIntent')
        $(element).addClass('hoverIntent');
        e.stopPropagation();
    }
    else {
        $(element).removeClass('hoverIntent');
    }
}

function setElementBackground(element, backgroundColor) {
    element.style.backgroundColor = backgroundColor;
}
