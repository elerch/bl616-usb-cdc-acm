Based on usbd_cdc_acm in the Bouffalo Labs sdk example
======================================================


Supported Chips
---------------

|      CHIP        | Remark |
|:----------------:|:------:|
|BL702/BL704/BL706 |        |
|BL616/BL618       |        |
|BL808             |   D0 has no irq     |

Compile (note that BL616 is all that's tested)
----------------------------------------------

- BL616/BL618

```
docker run --rm -t -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f BOARD=bl616dk CHIP=bl616
```

- BL702/BL704/BL706

```
make CHIP=bl702 BOARD=bl702dk
```


- BL808

```
make CHIP=bl808 BOARD=bl808dk CPU_ID=m0
make CHIP=bl808 BOARD=bl808dk CPU_ID=d0
```

Flash
-----
```
docker run --rm --device /dev/ttyACM0 -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f flash BOARD=bl616dk CHIP=bl616 COMX=/dev/ttyACM0
```

With rootless podman:

```
podman run --annotation run.oci.keep_original_groups=1 --userns=keep-id --rm --device /dev/ttyACM0 -v $(pwd):/build git.lerch.org/lobo/bouffalo_open_sdk:2f6477f flash BOARD=bl616dk CHIP=bl616 COMX=/dev/ttyACM0
```
