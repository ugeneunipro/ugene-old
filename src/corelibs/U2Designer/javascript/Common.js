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
 
function wrapLongText(text) {
    return '<div class="long-text" title="' + text + '">' + text + '</div>';
}

/**
 * Hides the hint of the load scheme button and
 * specifies not to show it anymore.
 */
function hideLoadBtnHint() {
    var hint = document.getElementById("load-btn-hint");
    hint.setAttribute("style", "display: none");
    agent.hideLoadButtonHint();
}

/**
 * Shows a button to load the original scheme.
 * If "showHint" is "true", shows a hint about the button usage.
 */
function showLoadButton(showHint) {
    var menuLine = document.getElementsByClassName("dash-menu-line")[0];
    var btnDef = "<button class='btn load-btn' onclick='agent.loadSchema()' title='Load schema'><div /></button>";
    menuLine.insertAdjacentHTML('beforeend', btnDef);

    if (showHint == true) {
        var hintDef =
        "<div id='load-btn-hint-container'>" +
            "<div id='load-btn-hint' class='popover fade bottom in' style='display: block'>" +
                "<div class='arrow' style='left: 91%'></div>" +
                "<div class='popover-content'>" +
                    "You can always open the original workflow for your results by clicking on this button." +
                    "<div style='text-align: center;'>" +
                        "<button class='btn' onclick='hideLoadBtnHint()' style='margin-bottom: 4px; margin-top: 6px;'>OK, got it!</button>" +
                    "</div>" +
                "</div>" +
            "</div>" +
        "</div>";
        menuLine.insertAdjacentHTML('beforeend', hintDef);
    }
}

function showFileButton(url) {
    var fileName = url.slice(url.lastIndexOf('/') + 1, url.length);
    var path = url.slice(0, url.lastIndexOf('/') + 1);
    var button = 
        '<div class="file-button-ctn">' +
            '<div class="btn-group full-width file-btn-group">' + 
                '<button class="btn full-width long-text" onclick="agent.openUrl(\'' + url + '\')">' + fileName +
                '</button>' +
                    '<button class="btn dropdown-toggle" data-toggle="dropdown">' + 
                        '<span class="caret"></span>' +
                    '</button>' +
	            '<ul class="dropdown-menu full-width">' +
       	                '<li><a href="#" onclick="agent.openByOS(\'' + path + '\')">Open containing folder</a></li>' +
                        '<li><a href="#" onclick="agent.openByOS(\'' + path + fileName + '\')">Open by operating system</a></li>' + 
                    '</ul>' +
            '</div>' +
        '</div>';
	return button;
}

