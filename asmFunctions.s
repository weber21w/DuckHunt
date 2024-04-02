.global PaletteBlitPart

;***********************************
; SET TILE 8bit mode
; C-callable
; r24: RAM tile index (bt)
; r23:r22: Sprite flags : Sprite tile index
; r21:r20: Y:X (0 or 1, location of 8x8 sprite fragment on 2x2 tile container)
; r19:r18: DY:DX (0 to 7, offset of sprite relative to 0:0 of container)
;************************************
PaletteBlitSpritePart:
