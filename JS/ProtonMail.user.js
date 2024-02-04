// ==UserScript==
// @name        Proton Mail - Delete Helper
// @namespace   Proton Mail
// @match       https://mail.proton.me/u/0/*
// @grant       none
// @version     1.0
// @author      Dax
// @description 2/4/2024, 10:09:34 AM
// ==/UserScript==

function getSelectUnstarred() {
  let btnselectalldd = document.querySelector("button[data-testid='toolbar:select-all-dropdown']");
  if(!btnselectalldd) return null;

  btnselectalldd.click();
  let btnselectunstarred = document.querySelector("button[data-testid='toolbar:all-unstarred']");
  if(!btnselectunstarred) return null;

  btnselectunstarred.addEventListener("click", () => {
    btnselectalldd.click(); //Hide Dropdown
  });

  return btnselectunstarred;
}

function injectButton() {
  let btnsentinel = document.querySelector("button[data-testid='filter-dropdown:show-all']");
  if(!btnsentinel) return;

  let toolbar = btnsentinel.parentNode;
  if(!toolbar) return;

  let button = toolbar.appendChild(document.createElement("button"));
  button.className = "button button-small button-ghost-weak toolbar-button toolbar-button--small text-sm m-0 text-bold";
  button.innerText = "Delete Page";
  button.type = "button"

  button.addEventListener("click", () => {
    let btnselectunstarred = getSelectUnstarred();

    if(!btnselectunstarred) {
      console.error("[ProtonMail] Cannot find 'Select Unstarred' button");
      return;
    }

    btnselectunstarred.click();
    let btnmovetotrash = document.querySelector("button[data-testid='toolbar:movetotrash']");

    if(!btnmovetotrash) {
      console.error("[ProtonMail] Cannot find 'Trash' button");
      return;
    }

    btnmovetotrash.click();
  });
}

setTimeout(injectButton, 2000);
