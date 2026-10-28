# Metal on this uke (Tianma panel unit)

Known good on 2026-10-28: iPadOS 17.7.11 composited and presented by the Adreno 732 through the NativeAdrenoMetal plugin, with touch,
Setup skipped and no text over the panel.

## Kernel

`build-kernel.yml` on this branch with `uke-gpu-userland_defconfig` (run 40642143803, commit a9a3d56, kernel.macho sha256 5d020ecf...).
The device repository carries its own copies of ArmCpu, NativeAdreno and NativeMobileDisplay (they override the core's):

| Change | Why |
| --- | --- |
| ArmCpu acknowledges SGIs 1-15 | an SGI 7 arrives after GX power-on (mode 8) and panicked the stock handler |
| GpuPowerProbe names backboardd in NativeSandboxGrantProcess | backboardd's open of the GPU user client was denied by its sandbox |
| NativeAdreno keeps GPU write access for backboardd's read-only imports of 16 MiB or more | the compositor surface reaches backboardd read-only; GPU writes were context permission faults |
| NativeMobileDisplay presentBase offers the base frame to the GPU (sheng-fb-gpu=1) | the uke compositor never validates a swap surface, so the GPU scanout client got no frames |
| console muted on the first GPU frame | the text console kept drawing over the GPU's frames |
| UKE SMMU / STALL / ROIMPORT / ROMAP / SGI log lines | diagnostics read back through OrangeFox |

## Userland (built locally, not in CI)

Core `Allenwdk/UniXNU` uke-gpu 48b45aa, NativeMetal 6dcd4cb, Mesa 85fda3b3 with the core's recorded patches, metal2vulkan 720ef1ce,
Rust 1.98.0. Apply `native-metal-plugin.patch` (Turnip with `TU_DEBUG=nocb,sysmem`: the A732 hung in BV binning) and
`core-scripts.patch` (host-sim fixes; seed tools read only the APFS header instead of the whole 95 GiB image), then in the tree:

    python3 scripts/build-turnip-darwin.py --target ios
    python3 scripts/build-turnip-darwin.py --target host
    python3 scripts/build-native-metal-bridge.py --target ios
    python3 scripts/build-adreno-metal-plugin.py
    python3 scripts/make-owner-trust-bundle.py
    python3 scripts/add-gpu-plugin-data.py --publish <skip-setup seed> <gpu seed>

## Payload

`size_mib = 94` for this build only (the payload is 88 MiB; the vendor_boot partition is 96 MiB and the image ends 196 KiB short of it).
Kernel args: `offline-legacy-base-present=1 offline-legacy-noconsole=1 sheng-touch=4 sheng-touch-hid=1 sheng-gpu-power=9
sheng-gpu-metal=1 sheng-fb-gpu=1 sheng-gpu-addon=1`, `--owner-trust-cache` with the plugin's trust bundle, the GPU seed as `--data-ramdisk`.
