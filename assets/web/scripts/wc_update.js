/*
From the user input, we have to read:
    title
    description
    date
    distribution info/count
    pokemon icons
*/

let title = document.getElementById("create-view-title");
let description = document.getElementById("create-view-desc");
let date = document.getElementById("create-view-date");
let distrib = document.getElementById("create-view-distrib");

let icon_left = document.getElementById("create-view-icon-left");
let icon_middle = document.getElementById("create-view-icon-middle");
let icon_right = document.getElementById("create-view-icon-right");

//We need their sources

let title_src = document.getElementById("title_input");
let description_src = document.getElementById("desc_input");

let date_src = document.getElementById("wc_date");
let date_current_src = document.getElementById("current_date");
let date_current_viewable = document.getElementById("current_date_viewable");

let distrib_src = document.getElementById("distrib_input");
let distrib_infinite_src = document.getElementById("distrib_infinite");

let icon_left_src = document.getElementById("pokemon_container_left");
let icon_middle_src = document.getElementById("pokemon_container_middle");
let icon_right_src = document.getElementById("pokemon_container_right");

//To make life easier, we will rely on the view button to update
let view_button = document.getElementById("view-button");

//Function for converting a date into the wc format
const transformDate = (input_date) => {
    if (!input_date) {
        return "Jan. 01, 2000";
    }

    //JS dates in the format YYYY-MM-DD
    const date_obj = new Date(input_date);

    let output_date = "";
    const day = date_obj.getDate();
    const month = date_obj.getMonth();
    const year = date_obj.getFullYear();
    
    const months = [
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec"
    ];

    output_date = `${months[month]}. ${day}, ${year}`;

    return output_date;
};


//detect a click
view_button.onclick = () => {
    //We want to read the value stored in each src, and put it into the html.
    let title_text = title_src.value;
    title.innerText = title_text;

    let desc_text = description_src.value;
    description.innerText = desc_text;

    let date_text = date_src.value;
    let date_current_value = date_current_src.checked;
    
    if (date_current_value) {
        //We have to read the innerText of date_current_src,
        //find the brackets, remove them, then pass them in.
        let date_current_text = date_current_viewable.innerText;
        let date_current = date_current_text.substring(date_current_text.indexOf("("));
        date_current = date_current.slice(1, date_current.length - 1);

        date.innerText = transformDate(date_current);
    }
    else {
        date.innerText = transformDate(date_text);
    }

    let icon_left_text = icon_left_src.innerHTML;
    //We want to cut off the pokemon name.
    let icon_left_proper = icon_left_text.substring(0, icon_left_text.indexOf("</span>")) + "</span>";
    icon_left.innerHTML = icon_left_proper;

    let icon_right_text = icon_right_src.innerHTML;
    //We want to cut off the pokemon name.
    let icon_right_proper = icon_right_text.substring(0, icon_right_text.indexOf("</span>")) + "</span>";
    icon_right.innerHTML = icon_right_proper;

    let icon_middle_text = icon_middle_src.innerHTML;
    //We want to cut off the pokemon name.
    let icon_middle_proper = icon_middle_text.substring(0, icon_middle_text.indexOf("</span>")) + "</span>";
    icon_middle.innerHTML = icon_middle_proper;
    
    let distrib_text = distrib_src.value;
    let distrib_infinite_value = distrib_infinite_src.checked;
    if (distrib_infinite_value || distrib_text == 255) {
        distrib.innerHTML = "This gift can be shared with as\nmany friends as you like.";
    }
    else if (distrib_text == 0) {
        distrib.innerHTML = "";
    }
    else {
        distrib.innerHTML = `This gift can be distributed ${distrib_text} time(s).`
    }
};

