// Get inputs
const upload_button = document.getElementById("pokemon_data_upload");
const raw_data = document.getElementById("pokemon_data_raw");

// Listen for a file upload
upload_button.addEventListener("change", (b_event) => {
    const file = b_event.target.files[0];
    if (!file) return;

    const reader = new FileReader();

    // Triggers upon finished load (file read)
    reader.onload = (r_event) => {
        // JS stores raw binary in an ArrayBuffer
        const raw_bin = r_event.target.result;

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
        //const url_encrypted_string = encrypted_string;

        raw_data.value = url_encrypted_string;
        console.log(url_encrypted_string);
        console.log("hello");
    }

    //Finally, read the data uploaded
    reader.readAsArrayBuffer(file);

});