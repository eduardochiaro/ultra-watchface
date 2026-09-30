// Values match ComplicationId in src/c/complications/complications.h.
var SLOTS = [
  { label: 'Steps', value: '1' },
  { label: 'Temperature', value: '2' },
  { label: 'Battery', value: '3' },
  { label: 'Chance of rain', value: '4' },
  { label: 'None', value: '0' }
];

function slot(key, label, def) {
  return { type: 'select', messageKey: key, label: label, defaultValue: def, options: SLOTS };
}

module.exports = [
  { type: 'heading', defaultValue: 'Ultra' },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Complications' },
      slot('SLOT_TL', 'Top left', '1'),
      slot('SLOT_TR', 'Top right', '2'),
      slot('SLOT_BL', 'Bottom left', '3'),
      slot('SLOT_BR', 'Bottom right', '4')
    ]
  },
  {
    type: 'section',
    items: [
      { type: 'heading', defaultValue: 'Options' },
      {
        type: 'select', messageKey: 'UNITS', label: 'Temperature', defaultValue: '0',
        options: [{ label: 'Celsius', value: '0' }, { label: 'Fahrenheit', value: '1' }]
      },
      {
        type: 'slider', messageKey: 'STEP_GOAL', label: 'Daily step goal',
        defaultValue: 10000, min: 1000, max: 30000, step: 500
      },
      {
        type: 'toggle', messageKey: 'SECONDS', label: 'Seconds hand',
        description: 'Uses more battery', defaultValue: false
      }
    ]
  },
  { type: 'submit', defaultValue: 'Save' }
];
