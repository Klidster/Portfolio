const menuToggle = document.getElementById("menu-toggle");
const navLinks = document.querySelector(".nav-links");

if (menuToggle && navLinks) {
  menuToggle.addEventListener("click", () => {
    navLinks.classList.toggle("show");
  });
}

const itemToggles = document.querySelectorAll(".item-toggle");

itemToggles.forEach((toggle) => {
  toggle.addEventListener("click", () => {
    const card = toggle.parentElement;
    card.classList.toggle("active");
  });
});