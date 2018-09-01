document.addEventListener('DOMContentLoaded', () => {
  'use strict';

  let target = document.getElementById('temperature');

  setInterval(() => {
    fetch('/fahrenheit').then(response => {
      return response.json();
    }).then(content => {
      while(target.firstChild) {
        target.removeChild(target.firstChild);
      }
      target.appendChild(document.createTextNode(content.temperature));
    });
  }, 1000);
});
