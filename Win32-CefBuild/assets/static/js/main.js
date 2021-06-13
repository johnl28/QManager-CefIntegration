/*
File: app.js
Author: johnl
Date: 02.06.2021
*/  

var selected = 0

var pages = [
	{
		name : "Home",
		span : "home",
		page: "home.html",
		disabled: false,
	},
	{
		name : "Icon Manager",
		span : "wallpaper",
		page: "desktopManager.html",
		disabled: false,
	},
	{
		name : "Desktop Personalisation",
		span : "brush",
		page: "desk-design.html",
		disabled: false,
	},
	{
		name : "Shutdown Manager",
		span : "power_settings_new",
		disabled: true,
	},
	{
		name : "Settings",
		span : "settings",
		page: "settings.html",
		disabled: false,
	},

];

function Exit()
{
	Quit();
}

function SetLoadingState(isLoading)
{
	if(isLoading)
	{
		$("#title-icon").attr("src","img/icon/icon.gif");
		return;
	}

	$("#title-icon").attr("src","img/icon/icon.png");
}

function togglePage(page, index)
{
	if(index == selected) {
		return;
	}

	selected = index;
	$('#router').load(`pages/${page}`, function () {
		InitCommon(); // init common lib
	});
}


$(document).ready(OnReady);
function OnReady()
{
	$("#title-label").text(window.GetTitle());

	for (var i = 0; i < pages.length; i++) 
	{
		if(!pages[i].disabled) {
			$(".nav").append(`<button onclick="togglePage('${pages[i].page}', ${i})"><span class="material-icons">${pages[i].span}</span></button>`);
		}
	}

	$('#router').load(`pages/home.html`);
	InitCommon(); // init common lib
}





