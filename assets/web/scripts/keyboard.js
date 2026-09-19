/*
To be put in wc_text.html
*/

//Data from HTML to process
const title_input = document.getElementById("title_input");
const desc_input = document.getElementById("desc_input");
const keyboard_buttons = document.querySelectorAll("#keyboard-button"); //# -> id

//Default focused text box
let focused_input = title_input;

//Update based on current focus
[title_input, desc_input].forEach(input => {
    input.addEventListener("focus", () => {
        focused_input = input;
    });
});

char_count = focused_input.value.len;
//Process each keyboard button to display clicked character
keyboard_buttons.forEach(button => {
    //We want to ignore mousedown (any button on the mouse)
    button.addEventListener("mousedown", (event) => {
        event.preventDefault();
    });
    //Instead, listen for click (left mouse down then up)
    button.addEventListener("click", () => {
        let char_count = focused_input.value.length;
        const char_max = focused_input.getAttribute("maxlength");
        
        //console.log(`focused_input: ${focused_input.value}`);
        //console.log(`char_count: ${char_count} char_max: ${char_max}`);
        
        if (!focused_input) return; //Don't do anything if we are not typing
        if (char_count >= char_max) return; //Keep within character max

        //Get character stored in button
        const curChar = button.getAttribute("data-char");
        
        console.log(`0x${curChar.charCodeAt(0).toString(16)}`);
        
        //Insert the character at the end of the current string
        focused_input.setRangeText(
            curChar,
            focused_input.selectionStart,
            focused_input.selectionEnd,
            "end"
        );

        //Re-focus the text box
        focused_input.focus();

        //Trigger input for live feed
        focused_input.dispatchEvent(new Event("input", { bubbles: true }));
    });
});