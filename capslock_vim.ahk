#Requires AutoHotkey v2.0

; ------------------------------
; CapsLock Dual-Role
; Tap (<120ms) -> Esc
; Hold -> Symbol Layer
; ------------------------------

threshold := 0.12
capsHeld := false

$*CapsLock::
{
    global threshold, capsHeld

    if !KeyWait("CapsLock", "T" threshold)
    {
        capsHeld := true
        KeyWait("CapsLock")
        capsHeld := false
        return
    }

    Send("{Esc}")
}

; ------------------------------
; Symbol Layer (aktiv solange Caps gehalten wird)
; ------------------------------

#HotIf capsHeld

; ()
j::Send("{Raw}(")
k::Send("{Raw})")

; {}
u::Send("{Raw}{")
i::Send("{Raw}}")

; []
m::Send("{Raw}[")
,::Send("{Raw}]")

; <>
h::Send("{Raw}<")
l::Send("{Raw}>")

; =
ö::Send("{Raw}=")

; ->
n::Send("{Raw}->")

#HotIf

; CapsLock LED immer aus
SetCapsLockState("AlwaysOff")