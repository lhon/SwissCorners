var complicationOptions = [
  { "label": "Off", "value": "off" },
  { "label": "Temperature", "value": "temp" },
  { "label": "Steps", "value": "steps" },
  { "label": "Day (Mon, Tue…)", "value": "day" },
  { "label": "Date (1–31)", "value": "date" },
  { "label": "Heart Rate", "value": "hr" },
  { "label": "Sunrise", "value": "sunrise" },
  { "label": "Sunset", "value": "sunset" },
  { "label": "Weather Icon", "value": "weather" }
];

module.exports = [
  {
    "type": "heading",
    "defaultValue": "ModernSwiss Settings"
  },
  {
    "type": "text",
    "defaultValue": "Customize your Swiss-style watchface."
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Dial Appearance"
      },
      {
        "type": "select",
        "messageKey": "DialColor",
        "defaultValue": "round",
        "label": "Dial Color",
        "options": [
          {
            "label": "White Dial",
            "value": "white"
          },
          {
            "label": "Black Dial",
            "value": "black"
          },
          {
            "label": "White Dial (No Logo)",
            "value": "white_nl"
          },
          {
            "label": "Black Dial (No Logo)",
            "value": "black_nl"
          },
          {
            "label": "Round (No Logo)",
            "value": "round"
          }
        ]
      },
      {
        "type": "color",
        "messageKey": "RoundBackgroundColor",
        "defaultValue": "FFFFFF",
        "label": "Round background color (square Pebble only)",
        "description": "Background color shown around the round face on square Pebble watches"
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Complications"
      },
      {
        "type": "text",
        "defaultValue": "Choose what appears in each corner. Use a light color on black dials."
      },
      {
        "type": "select",
        "messageKey": "CompTL",
        "defaultValue": "temp",
        "label": "Top Left",
        "options": complicationOptions
      },
      {
        "type": "color",
        "messageKey": "CompTLColor",
        "defaultValue": "000000",
        "label": "Top Left Color"
      },
      {
        "type": "select",
        "messageKey": "CompTR",
        "defaultValue": "day",
        "label": "Top Right",
        "options": complicationOptions
      },
      {
        "type": "color",
        "messageKey": "CompTRColor",
        "defaultValue": "000000",
        "label": "Top Right Color"
      },
      {
        "type": "select",
        "messageKey": "CompBL",
        "defaultValue": "steps",
        "label": "Bottom Left",
        "options": complicationOptions
      },
      {
        "type": "color",
        "messageKey": "CompBLColor",
        "defaultValue": "000000",
        "label": "Bottom Left Color"
      },
      {
        "type": "select",
        "messageKey": "CompBR",
        "defaultValue": "date",
        "label": "Bottom Right",
        "options": complicationOptions
      },
      {
        "type": "color",
        "messageKey": "CompBRColor",
        "defaultValue": "000000",
        "label": "Bottom Right Color"
      },
      {
        "type": "select",
        "messageKey": "TempUnit",
        "defaultValue": "c",
        "label": "Temperature Unit",
        "options": [
          { "label": "Celsius (°C)", "value": "c" },
          { "label": "Fahrenheit (°F)", "value": "f" }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Second Hand"
      },
      {
        "type": "select",
        "messageKey": "SecondHandOption",
        "defaultValue": "quartz",
        "label": "Second Hand Style",
        "options": [
          {
            "label": "Quartz",
            "value": "quartz"
          },
          {
            "label": "Stop2Go",
            "value": "stop2go"
          },
          {
            "label": "Off",
            "value": "off"
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Date Display"
      },
      {
        "type": "select",
        "messageKey": "DateOption",
        "defaultValue": "nodate",
        "label": "Date Window (on dial)",
        "options": [
          {
            "label": "No Date",
            "value": "nodate"
          },
          {
            "label": "Show Date",
            "value": "showdate"
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Hourly Vibration"
      },
      {
        "type": "select",
        "messageKey": "HourlyVibration",
        "defaultValue": "off",
        "label": "Hourly Vibration",
        "options": [
          {
            "label": "Off",
            "value": "off"
          },
          {
            "label": "Short Pulse",
            "value": "short"
          },
          {
            "label": "Long Pulse",
            "value": "long"
          },
          {
            "label": "Double Pulse",
            "value": "double"
          }
        ]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Bluetooth"
      },
      {
        "type": "toggle",
        "messageKey": "BluetoothStatusDetection",
        "label": "Show Bluetooth Status",
        "defaultValue": false,
        "description": "Show splash screen and vibrate on connect/disconnect"
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];
