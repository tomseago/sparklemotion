import React, { Component } from 'react';
import styles from './ColorPicker.scss';
import classNames from 'classnames';
import chroma from 'chroma-js';
import throttle from 'lodash/throttle';
import Draggable from 'react-draggable';
import { rgb2xy, xy2polar, xy2rgb } from '../../../../utils/colorUtils';

const baaahs = sparklemotion.baaahs;

const pickerRadius = 12;

class ColorPicker extends Component {
  constructor(props) {
    super(props);

    const { gadget } = this.props;
    const { redI, greenI, blueI } = gadget.color;
    const color = [redI, greenI, blueI];

    this.state = {
      gadget,
      color,
      radius: 200,
      clientWidth: 400,
    };

    // this._updateColorStateFromGadget(gadget);
    // Send color updates once every 150ms while the picker is dragged
    this._throttledHandleColorChange = throttle(this._handleColorChange, 150);

    gadget.listen({
      onChanged: (gadget) => {
        const { redI, greenI, blueI } = gadget.color;
        const color = [redI, greenI, blueI];
        this.setState({ color });
      },
    });
  }

  updateWheel() {
    const { radius } = this.state;
    if (!this._canvasEl || !radius) return;

    const width = radius * 2;
    const height = radius * 2;
    const feateringPx = 1;

    let ctx = this._canvasEl.getContext('2d');
    let image = ctx.createImageData(width, height);
    let data = image.data;

    for (let x = -radius; x < radius; x++) {
      for (let y = -radius; y < radius; y++) {
        let [r] = xy2polar(x, y);
        // Figure out the starting index of this pixel in the image data array.
        let rowLength = 2 * radius;
        let adjustedX = x + radius; // convert x from [-50, 50] to [0, 100] (the coordinates of the image data array)
        let adjustedY = y + radius; // convert y from [-50, 50] to [0, 100] (the coordinates of the image data array)
        let pixelWidth = 4; // each pixel requires 4 slots in the data array
        let index = (adjustedX + adjustedY * rowLength) * pixelWidth;

        let [red, green, blue] = xy2rgb(x, y, radius);
        let alpha = 255;
        if (r >= radius - feateringPx) {
          alpha = Math.max(
            255 - 255 * ((r - (radius - feateringPx)) / feateringPx),
            0
          );
        }

        data[index] = red;
        data[index + 1] = green;
        data[index + 2] = blue;
        data[index + 3] = alpha;
      }
    }

    ctx.putImageData(image, 0, 0);
  }

  _handleColorChange = () => {
    const { color } = this.state;
    const [red, green, blue] = color;
    const hex = chroma.rgb(red, green, blue).hex();

    const { gadget } = this.state;
    gadget.color = baaahs.Color.Companion.fromString(hex);
    this.setState({ gadget, color });
  };

  _onPickerMouseDown(ev) {
    this.setState({ selected: true });
  }

  _onPickerDragged(ev, data) {
    const { x, y } = data;
    const { radius } = this.state;

    const [red, green, blue] = xy2rgb(x - radius, y - radius, radius);

    this.setState({ color: [red, green, blue] }, () =>
      this._throttledHandleColorChange()
    );
  }

  _onPickerMouseUp(ev, data) {
    const { radius } = this.state;
    const color = xy2rgb(data.x - radius, data.y - radius, radius);

    this.setState(
      {
        color,
        selected: false,
      },
      () => this._handleColorChange()
    );
  }

  render() {
    const { radius, selected, color } = this.state;
    const [r, g, b] = color;

    const pickerPosition = rgb2xy(color, radius);

    return (
      <div
        ref={(el) => {
          if (!el) return;

          const { clientWidth } = el;
          const newRadius = clientWidth / 2;
          if (radius != newRadius) {
            this.setState({ radius: newRadius }, () => this.updateWheel());
          }
        }}
        className={styles['color-picker--pad']}
        style={{ height: radius * 2 }}
      >
        <canvas
          width={radius * 2}
          height={radius * 2}
          className={styles['canvas']}
          ref={(el) => (this._canvasEl = el)}
        />
        {radius != null && (
          <Draggable
            position={{ x: pickerPosition[0], y: pickerPosition[1] }}
            onStart={(e, data) => {
              return this._onPickerMouseDown(e, data);
            }}
            onDrag={(e, data) => {
              return this._onPickerDragged(e, data);
            }}
            onStop={(e, data) => {
              return this._onPickerMouseUp(e, data);
            }}
          >
            <div>
              <div
                style={{
                  width: pickerRadius * 2,
                  height: pickerRadius * 2,
                  backgroundColor: `rgb(${r}, ${g}, ${b})`,
                }}
                className={classNames(styles['color-picker--picker'], {
                  [styles['selected']]: selected,
                })}
              />
            </div>
          </Draggable>
        )}
      </div>
    );
  }
}

export default ColorPicker;
