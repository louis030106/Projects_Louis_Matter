const toggle = document.querySelector(".nav-toggle");
const links = document.querySelector(".nav-links");

toggle.addEventListener("click", () => {
  links.classList.toggle("open");
});

document.querySelectorAll(".nav-links a").forEach(link => {
  link.addEventListener("click", () => {
    links.classList.remove("open");
  });
});

const modal = document.getElementById("modal");
const modalImg = document.getElementById("modalImg");
const modalClose = document.getElementById("modalClose");

function closeModal() {
  modal.classList.remove("open");
  modalImg.src = "";
  modalImg.alt = "";
}

document.querySelectorAll(".gallery-item").forEach(img => {
  img.addEventListener("click", () => {
    modalImg.src = img.dataset.full;
    modalImg.alt = img.alt;
    modal.classList.add("open");
  });
});

modalClose.addEventListener("click", closeModal);

modal.addEventListener("click", event => {
  if (event.target === modal) {
    closeModal();
  }
});

document.addEventListener("keydown", event => {
  if (event.key === "Escape") {
    closeModal();
  }
});