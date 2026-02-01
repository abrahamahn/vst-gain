Parameter System (Beginner-Friendly)
===================================

What is a "parameter"?
----------------------
A parameter is any value you want the user to control and the DAW to automate.
Examples: gain, cutoff, mix, attack, release.

Web-dev analogy:
- Parameter list = the "schema" of your plugin state.
- Slider/knob = UI bound to a field in that schema.
- Preset = a saved JSON object of those values.

Where it lives in this boilerplate
----------------------------------
- `src/infra/parameters/ParameterRegistry.*`
  - This is the single source of truth.
  - Every parameter is defined in one list.

How it works
------------
1) `ParameterRegistry` defines all parameters once.
2) `createLayout()` converts that list into JUCE parameters.
3) The processor pulls values with `getRawParameterValue()`.
4) The UI binds sliders to parameters using APVTS attachments.

Adding a new parameter (example)
--------------------------------
Example: Output Trim (already added)

1) Add it to `ParameterRegistry.cpp`:
   - id: `trim`, name: `Output Trim`, range: `-12..+12 dB`
2) Create a slider in `PluginEditor.cpp`.
3) Attach the slider to the new parameter id.
4) Use it in `processBlock()` to convert dB to gain.

Why this is "robust"
---------------------
- Everything is defined once.
- UI, audio, and presets can’t drift out of sync.
- Easy to add parameters without touching multiple files.
