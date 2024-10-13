"use strict"

// correct result: the rendered triangle is red
// wrong result: the rendered triangle is teal

// NOTE: switching the binding around makes it work, but it should have an effect
const use_dynamic_offsets = true;
const red_binding = 1;
const grey_binding = 0;

const ub_align = 256;
const ub_size = 2 * ub_align;

const red = [ 1.5, 0, 0, 0 ];
const grey = [ 0.5, 0.5, 0.5, 0 ];

const red_offset = 0;
const grey_offset = ub_align;

const vs = `
@vertex fn main(@builtin(vertex_index) VertexIndex : u32) -> @builtin(position) vec4f {
  const pos = array<vec2f, 3>(
    vec2(0.0, 0.5),
    vec2(-0.5, -0.5),
    vec2(0.5, -0.5)
  );
  return vec4f(pos[VertexIndex], 0.0, 1.0);
}`;

const fs = `
@group(0) @binding(${red_binding}) var<uniform> red: vec4f;
@group(0) @binding(${grey_binding}) var<uniform> grey: vec4f;
@fragment fn main() -> @location(0) vec4f {
  return saturate(vec4f(red.xyz - grey.xyz, 1.0));
}`;

const canvas = document.querySelector('canvas');
const adapter = await navigator.gpu.requestAdapter();
const wgpu = await adapter.requestDevice();
const context = canvas.getContext('webgpu');
const presentationFormat = navigator.gpu.getPreferredCanvasFormat();
context.configure({ device: wgpu, format: presentationFormat });

const ub = wgpu.createBuffer({
  size: ub_size,
  usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST | GPUBufferUsage.COPY_SRC,
  mappedAtCreation: true,
});

// write red vec4f at byte offset 0 and grey vec4 at byte offset 256
const ub_data = new Float32Array(ub.getMappedRange());
ub_data.set(red, red_offset / Float32Array.BYTES_PER_ELEMENT);
ub_data.set(grey, grey_offset / Float32Array.BYTES_PER_ELEMENT);
ub.unmap();

const bgl = wgpu.createBindGroupLayout({
  entries: [
    {
      binding: red_binding,
      visibility: GPUShaderStage.FRAGMENT,
      buffer: {
        type: 'uniform',
        hasDynamicOffset: use_dynamic_offsets,
      }
    },
    {
      binding: grey_binding,
      visibility: GPUShaderStage.FRAGMENT,
      buffer: {
        type: 'uniform',
        hasDynamicOffset: use_dynamic_offsets,
      }
    }
  ]
});

const bg = wgpu.createBindGroup({
  layout: bgl,
  entries: [
    {
      binding: red_binding,
      resource: {
        buffer: ub,
        offset: use_dynamic_offsets ? 0 : red_offset,
        size: 16,
      }
    },
    {
      binding: grey_binding,
      resource: {
        buffer: ub,
        offset: use_dynamic_offsets ? 0 : grey_offset,
        size: 16,
      }
    }
  ]
});

const pip = wgpu.createRenderPipeline({
  layout: wgpu.createPipelineLayout({
    bindGroupLayouts: [bgl],
  }),
  vertex: {
    module: wgpu.createShaderModule({
      code: vs,
    }),
  },
  fragment: {
    module: wgpu.createShaderModule({
      code: fs,
    }),
    targets: [
      {
        format: presentationFormat,
      }
    ],
  },
  primitive: {
    topology: 'triangle-list',
  },
});

const frame = () => {
  const cmdEncoder = wgpu.createCommandEncoder();
  const pass_desc = {
    colorAttachments: [
      {
        view: context.getCurrentTexture().createView(),
        clearValue: [0.5, 0.5, 0.5, 1.0],
        loadOp: 'clear',
        storeOp: 'store',
      }
    ]
  }
  const passEncoder = cmdEncoder.beginRenderPass(pass_desc);
  passEncoder.setPipeline(pip);

  if (use_dynamic_offsets) {
    passEncoder.setBindGroup(0, bg, [red_offset, grey_offset]);
  } else {
    passEncoder.setBindGroup(0, bg);
  }

  passEncoder.draw(3);
  passEncoder.end();

  wgpu.queue.submit([cmdEncoder.finish()]);
  requestAnimationFrame(frame);
}
requestAnimationFrame(frame);