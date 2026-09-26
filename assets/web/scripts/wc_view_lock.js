
const lockCardScale = () => {
    
    //Use the same transform scale in the css
    const baseScale_card = 2;

    const baseScale_button = 1;
    const button_top_base = 20;
    const button_right_base = 20;


    //Get current zoom level of browser
    const zoomLevel = window.devicePixelRatio;

    //Apply the transformation to members of the create view

    //Card
    const card = document.querySelector(".create-view-wrapper");
    if (card) {
        card.style.transform = `scale(${baseScale_card/zoomLevel})`;
    }

    //Button
    const button = document.querySelector(".create-button");
    if (button) {
        button.style.transform = `scale(${baseScale_button/zoomLevel})`;
        
        button.style.top = `${button_top_base/zoomLevel}`;
        button.style.right = `${button_right_base/zoomLevel}`;
    }
}


//Run on page load
lockCardScale();

//Run on resize
window.addEventListener("resize", lockCardScale);