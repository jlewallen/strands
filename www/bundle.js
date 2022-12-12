function createColorGradient(canvas) {
  const ctx = canvas.getContext("2d");
  const gradient = ctx.createLinearGradient(0, 0, canvas.width - 1, 0);
  gradient.addColorStop(0, "rgb(255,   0,   0)");
  gradient.addColorStop(0.16, "rgb(255,   0, 255)");
  gradient.addColorStop(0.33, "rgb(0,     0, 255)");
  gradient.addColorStop(0.49, "rgb(0,   255, 255)");
  gradient.addColorStop(0.66, "rgb(0,   255,   0)");
  gradient.addColorStop(0.82, "rgb(255, 255,   0)");
  gradient.addColorStop(1, "rgb(255,   0,   0)");
  return gradient;
}

function createTransparentGradient(canvas) {
  const ctx = canvas.getContext("2d");
  const gradient = ctx.createLinearGradient(0, 0, 0, canvas.height - 1);
  gradient.addColorStop(0, "rgba(255, 255, 255, 1)");
  gradient.addColorStop(0.48, "rgba(255, 255, 255, 0)");
  gradient.addColorStop(0.52, "rgba(0,     0,   0, 0)");
  gradient.addColorStop(1, "rgba(0,     0,   0, 1)");
  return gradient;
}

function createColorCanvas() {
  const canvas = document.getElementById("color-canvas");
  const ctx = canvas.getContext("2d");
  ctx.fillStyle = createColorGradient(canvas);
  ctx.fillRect(0, 0, canvas.width - 1, canvas.height - 1);
  ctx.fillStyle = createTransparentGradient(canvas);
  ctx.fillRect(0, 0, canvas.width - 1, canvas.height - 1);
  return canvas;
}

window.addEventListener("DOMContentLoaded", (event) => {
  const colorCanvas = createColorCanvas();

  colorCanvas.addEventListener("click", (event) => {
    const imageData = colorCanvas
      .getContext("2d")
      .getImageData(event.offsetX, event.offsetY, 1, 1);
    const rgb = [imageData.data[0], imageData.data[1], imageData.data[2]];
    const selectedColor = "rgb(" + rgb[0] + "," + rgb[1] + "," + rgb[2] + ")";
    const integer = rgb[0] * 65536 + rgb[1] * 256 + rgb[2];

    console.log(
      "click: " + event.offsetX + ", " + event.offsetY + ", " + selectedColor
    );

    $("#color-value").val(selectedColor);

    submitVal("c", integer);
  });

  function onPalette(event, palette) {
    event.preventDefault();
    $("#palettes .active").removeClass("active");
    $(event.target).addClass("active");
    submitVal("p", palette);
  }

  $.ajax({
    url: "palettes.json",
  }).then((response) => {
    console.log("palettes", response);
    $("#palettes").append(
      response.palettes.map((row, index) =>
        $('<a href="#">' + row.name + "</a>")
          .click((event) => {
            console.log("palette", index, row);
            onPalette(event, index);
          })
          .wrap("<li></li>")
          .parent()
      )
    );
  });

  function onMode(event, mode) {
    event.preventDefault();
    $("#modes .active").removeClass("active");
    $(event.target).addClass("active");
    submitVal("m", mode);
  }

  $.ajax({
    url: "modes.json",
  }).then((response) => {
    console.log("modes", response);
    $("#modes").append(
      response.modes.map((row, index) =>
        $('<a href="#">' + row.name + "</a>")
          .click((event) => {
            console.log("mode", index, row);
            onMode(event, index);
          })
          .wrap("<li></li>")
          .parent()
      )
    );
  });
});

function onBrightness(event, dir) {
  event.preventDefault();
  submitVal("b", dir);
}

function onSpeed(event, dir) {
  event.preventDefault();
  submitVal("s", dir);
}

function submitVal(name, val) {
  $.ajax({
    url: "set?" + name + "=" + val,
  }).then(() => {
    console.log("ok");
  });
}
