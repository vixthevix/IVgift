export const base64_encode = (raw_bin) => {
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