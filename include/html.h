#include <Arduino.h>
#include <Logger.h>

const char index_html[] PROGMEM = R"rawliteral(
  <!DOCTYPE HTML>
  <html>
    <head>
      <title>S3XY Virtural Button Control</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link rel="icon" href="data:,">
      <style>
        html {font-family: Arial; display: inline-block; text-align: center;}
        h2 {font-size: 3.0rem;}
        p {font-size: 3.0rem;}
        body {max-width: 600px; margin:0px auto; padding-bottom: 25px;}

        div {
          margin: 0 auto;
          text-align: center;
          display: flex;
          align-items: center;
          justify-content: center;
          padding: 10px;
        }
        .switch-wrapper input[type="checkbox"] {
          visibility: hidden;
          display: none;
        }

        .switch-wrapper .toggle {
          position: relative;
          display: block;
          width: 70px;
          height: 40px;
          cursor: pointer;
          -webkit-tap-highlight-color: transparent;
          transform: translate3d(0, 0, 0);
        }

        .switch-wrapper .toggle:before {
          content: "";
          position: relative;
          top: 1px;
          left: 1px;
          width: 68px;
          height: 38px;
          display: block;
          background: #c8ccd4;
          border-radius: 20px;
          transition: background 0.2s ease;
        }

        .switch-wrapper .toggle span {
          position: absolute;
          top: 0;
          left: 0;
          width: 40px;
          height: 40px;
          display: block;
          background: #fff;
          border-radius: 25px;
          box-shadow: 0 2px 6px rgba(154,153,153,0.75);
          transition: all 0.2s ease;
        }

        .switch-wrapper .toggle span svg {
          width: 16.7px;
          height: 16.7px;
          margin: 11.7px;
          fill: none;
        }

        .switch-wrapper .toggle span svg path {
          stroke: #c8ccd4;
          stroke-width: 2;
          stroke-linecap: round;
          stroke-linejoin: round;
          stroke-dasharray: 24;
          stroke-dashoffset: 0;
          transition: all 0.5s linear;
        }

        .switch-wrapper input[type="checkbox"]:checked + .toggle:before {
          background: #474bff;
        }

        .switch-wrapper input[type="checkbox"]:checked + .toggle span {
          transform: translateX(30px);
        }

        .switch-wrapper input[type="checkbox"]:checked + .toggle span path {
          stroke: #474bff;
          stroke-dasharray: 25;
          stroke-dashoffset: 25;
        }

        .pure-button {
          cursor: pointer;
          display: inline-block;
          line-height: normal;
          vertical-align: middle;
          white-space: nowrap;
          -webkit-user-drag: none;
          box-sizing: border-box;
          user-select: none;
          background-color: #e6e6e6;
          border: #0000;
          border-radius: 2px;
          color: #000c;
          padding: .5em 1em;
          height:80px;
          width:200px
        }

        .button-success,
        .button-error,
        .button-warning,
        .button-secondary {
            color: white;
            border-radius: 4px;
            text-shadow: 0 1px 1px rgba(0, 0, 0, 0.2);
        }

        .button-success {
            background: rgb(28, 184, 65);
            /* this is a green */
        }

        .button-error {
            background: rgb(202, 60, 60);
            /* this is a maroon */
        }

        .button-warning {
            background: rgb(223, 117, 20);
            /* this is an orange */
        }

        .button-secondary {
            background: rgb(66, 184, 221);
            /* this is a light blue */
        }
      </style>
    </head>
    <body>
      <h3>S3XY Virtural Button Control</h3>
      <h4>Power Switch</h4>
      <div class="switch-wrapper">
        <input type="checkbox" id="switch-yoyo"  onchange="callUrl('/toggle_power')" %POWER_SWITCH%/ >
        <label for="switch-yoyo" class="toggle" >
          <span>
            <svg viewBox="0 0 10 10">
              <path d="M5,1 L5,1 C2.790861,1 1,2.790861 1,5 L1,5 C1,7.209139 2.790861,9 5,9 L5,9 C7.209139,9 9,7.209139 9,5 L9,5 C9,2.790861 7.209139,1 5,1 L5,9 L5,1 Z"></path>
            </svg>
          </span>
        </label>
      </div>
      <div>
        <span>
          <button class="button-success pure-button"  onclick="callUrl('/click')">Single Click</button>
        </span>
      </div>
      <div>
        <span>
          <button class="button-secondary pure-button"  onclick="callUrl('/dbl_click')">Double Click</button>
        </span>
      </div>
      <div>
        <span>
          <button class="button-warning pure-button"  onclick="callUrl('/long_click')">Long Click</button>
        </span>
      </div>
      <div>
        <span>
          <button class="button-error pure-button"  onclick="callUrl('/block_switch')">Block Switch</button>
        </span>
      </div>
      <div>
        <p id="status">Waiting...</p>
      </div>
      <script>
        function delay(ms) {
          return new Promise(resolve => setTimeout(resolve, ms));
        }
          
        async function delay_clear() {
          await delay(5000); // Pause for 5 seconds
          document.getElementById("status").innerText = "Waiting..."; 
        };

        function callUrl(url) {
          document.getElementById("status").innerText = "Sent Command...";
          fetch(url)
            .then(response => response.text())
            .then(data => {
                document.getElementById("status").innerText = "Success: " + data;
            })
            .catch(error => {
                document.getElementById("status").innerText = "Error: " + error;
            });
          delay_clear();
        }
        
        
      </script>
    </body>
  </html>
)rawliteral";
