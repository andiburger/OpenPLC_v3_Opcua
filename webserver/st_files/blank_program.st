PROGRAM prog0
  VAR
    var_in : BOOL;
    var_out : BOOL;
  END_VAR
  VAR
    IN_StartButton AT %IX0.0 : BOOL;
END_VAR
VAR
    IN_StopButton AT %IX1.0 :  BOOL;
END_VAR

VAR
    OUT_MotorControl AT %QX2.0 : BOOL;
    OUT_StatusIndicator AT %QX3.0 : BOOL;
END_VAR
var_out := var_in;
OUT_MotorControl := IN_StartButton;
END_PROGRAM


CONFIGURATION Config0

  RESOURCE Res0 ON PLC
    TASK Main(INTERVAL := T#50ms,PRIORITY := 0);
    PROGRAM Inst0 WITH Main : prog0;
  END_RESOURCE
END_CONFIGURATION
