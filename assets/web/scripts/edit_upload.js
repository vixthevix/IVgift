//import { base64_encode } from "./base64_encode.js";

const base64_encode = (raw_bin) => {
        //Store each byte as an item in a list
        const bytes = new Uint8Array(raw_bin);
        let byte_string = "";
        for (let i = 0; i < bytes.byteLength; i++) {
            //Interpret each byte as a character and add it to the string
            byte_string += String.fromCharCode(bytes[i]);
        }
        //Base64 encode it.
        const encrypted_string = btoa(byte_string);

        //Due to existence of '=' and '+', we must URL encode as well.
        const url_encrypted_string = encodeURIComponent(encrypted_string);

        return url_encrypted_string;
}

document.addEventListener("DOMContentLoaded", () => {
    const edit_form = document.getElementById("edit_form");
    const file_upload = document.getElementById("wc_data_upload");
    const file_raw = document.getElementById("wc_data_raw");
    
    const edit_button = document.querySelector("#edit_button input");

    //First, when edit is clicked, trigger a file select.
    edit_button.addEventListener("click", (event) => {
        //Prevent default behaviour.
        event.preventDefault();
        //console.log("you have pressed edit");
        //Open file select
        file_upload.click();
    });

    //Second, listen for a file upload.
    file_upload.addEventListener("change", (b_event) => {
        const file = b_event.target.files[0];
        if (!file) return;

        const reader = new FileReader();

        //Upon finished file read
        reader.onload = (r_event) => {
            //Get and store the base64 encoding of the file
            const raw_bin = r_event.target.result;
            const url_encrypted_string = base64_encode(raw_bin);
            file_raw.value = url_encrypted_string;
            console.log(url_encrypted_string);
            console.log("hello");

            //submit automatically.
            edit_form.submit();
        }

        //Finally, read the data uploaded.
        reader.readAsArrayBuffer(file);
    });
});