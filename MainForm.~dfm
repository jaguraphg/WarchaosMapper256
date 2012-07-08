object FMain: TFMain
  Left = 614
  Top = 234
  Width = 903
  Height = 694
  Caption = 'Warchaos mapper utility 256x256. (c) Balbes [OverTime] v2.0.'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poDesktopCenter
  Scaled = False
  OnCreate = FormCreate
  OnDestroy = FormDestroy
  OnResize = BigMapTopLeftChanged
  DesignSize = (
    895
    667)
  PixelsPerInch = 96
  TextHeight = 13
  object BigMap: TDrawGrid
    Left = 0
    Top = 0
    Width = 631
    Height = 667
    Cursor = crArrow
    Align = alClient
    Color = 1315860
    ColCount = 256
    DefaultColWidth = 42
    DefaultRowHeight = 42
    FixedCols = 0
    RowCount = 256
    FixedRows = 0
    Options = [goDrawFocusSelected]
    ParentShowHint = False
    ShowHint = True
    TabOrder = 0
    OnDrawCell = BigMapDrawCell
    OnKeyDown = BigMapKeyDown
    OnMouseMove = BigMapMouseMove
    OnSelectCell = BigMapSelectCell
    OnTopLeftChanged = BigMapTopLeftChanged
  end
  object Panel3: TPanel
    Left = 631
    Top = 0
    Width = 264
    Height = 667
    Align = alRight
    BevelInner = bvRaised
    BevelOuter = bvLowered
    TabOrder = 1
    DesignSize = (
      264
      667)
    object LTCPPackets: TLabel
      Left = 174
      Top = 391
      Width = 6
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '0'
    end
    object Label2: TLabel
      Left = 6
      Top = 391
      Width = 73
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Append Pages:'
    end
    object Label1: TLabel
      Left = 107
      Top = 391
      Width = 65
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'TCP packets:'
    end
    object LAppendPages: TLabel
      Left = 81
      Top = 391
      Width = 6
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '0'
    end
    object Label3: TLabel
      Left = 37
      Top = 415
      Width = 173
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Select Network Interface here:'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -11
      Font.Name = 'Tahoma'
      Font.Style = [fsBold]
      ParentFont = False
    end
    object Label4: TLabel
      Left = 208
      Top = 391
      Width = 33
      Height = 13
      Anchors = [akTop, akRight]
      Caption = 'Socks:'
    end
    object LSocketsUsed: TLabel
      Left = 245
      Top = 391
      Width = 6
      Height = 13
      Anchors = [akTop, akRight]
      Caption = '0'
    end
    object Button1: TButton
      Left = 4
      Top = 268
      Width = 256
      Height = 21
      Anchors = [akTop, akRight]
      Caption = 'Generate empty map file'
      Enabled = False
      TabOrder = 0
      OnClick = Button1Click
    end
    object SkormitButton: TButton
      Left = 4
      Top = 294
      Width = 125
      Height = 21
      Anchors = [akTop, akRight]
      Caption = #1057#1082#1086#1088#1084#1080#1090#1100' HTML ...'
      TabOrder = 1
      OnClick = SkormitButtonClick
    end
    object Panel1: TPanel
      Left = 3
      Top = 3
      Width = 258
      Height = 258
      Anchors = [akTop, akRight]
      AutoSize = True
      TabOrder = 2
      object MiniMap: TImage
        Left = 1
        Top = 1
        Width = 256
        Height = 256
        Cursor = crCross
        ParentShowHint = False
        ShowHint = True
        OnClick = MiniMapClick
        OnMouseMove = MiniMapMouseMove
      end
      object MapBevel: TBevel
        Left = 100
        Top = 63
        Width = 33
        Height = 25
        Shape = bsFrame
      end
    end
    object BtnStart: TButton
      Left = 6
      Top = 322
      Width = 120
      Height = 65
      Anchors = [akTop, akRight]
      Caption = 'START'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 3
      OnClick = BtnStartClick
    end
    object Memo1: TMemo
      Left = 3
      Top = 520
      Width = 257
      Height = 105
      Anchors = [akTop, akRight, akBottom]
      TabOrder = 4
    end
    object BtnStop: TButton
      Left = 138
      Top = 322
      Width = 120
      Height = 65
      Anchors = [akTop, akRight]
      Caption = 'STOP'
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 5
      OnClick = BtnStopClick
    end
    object CBNetworkInterfaces: TComboBox
      Left = 3
      Top = 430
      Width = 257
      Height = 21
      Anchors = [akTop, akRight]
      ItemHeight = 13
      TabOrder = 6
      OnChange = CBNetworkInterfacesChange
    end
    object Panel2: TPanel
      Left = 4
      Top = 456
      Width = 256
      Height = 57
      Anchors = [akTop, akRight]
      BevelInner = bvRaised
      BevelOuter = bvLowered
      TabOrder = 7
      object LName: TLabel
        Left = 4
        Top = 2
        Width = 37
        Height = 16
        Caption = 'Name'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
      object LPlayer: TLabel
        Left = 4
        Top = 19
        Width = 46
        Height = 16
        Caption = 'Player'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object LLand: TLabel
        Left = 4
        Top = 36
        Width = 30
        Height = 16
        Caption = 'Land'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
    end
    object Button2: TButton
      Left = 171
      Top = 630
      Width = 89
      Height = 33
      Anchors = [akRight, akBottom]
      Caption = 'B Y E'
      TabOrder = 8
      OnClick = Button2Click
    end
    object MergeButton: TButton
      Left = 133
      Top = 294
      Width = 127
      Height = 21
      Anchors = [akTop, akRight]
      Caption = 'MERGE map.txt'
      TabOrder = 9
      OnClick = MergeButtonClick
    end
    object Button3: TButton
      Left = 4
      Top = 630
      Width = 125
      Height = 33
      Anchors = [akRight, akBottom]
      Caption = #1055#1077#1088#1077#1075#1085#1072#1090#1100' '#1074#1089#1077' '#1074' bmp'
      TabOrder = 10
      OnClick = Button3Click
    end
    object Button4: TButton
      Left = 242
      Top = 413
      Width = 16
      Height = 16
      Caption = 'R'
      TabOrder = 11
      OnClick = Button4Click
    end
  end
  object BmpProgress: TProgressBar
    Left = 3
    Top = 304
    Width = 609
    Height = 41
    Anchors = [akLeft, akTop, akRight]
    Min = 0
    Max = 512
    Step = 1
    TabOrder = 2
    Visible = False
  end
  object OpenDialogHTML: TOpenDialog
    Filter = #1087#1077#1088#1076#1077#1090#1100' '#1085#1077#1083#1100#1079#1103' (*.htm)|*.htm|'#1084#1086#1078#1085#1086' '#1087#1077#1088#1076#1077#1090#1100' (*.*)|*.*'
    Options = [ofHideReadOnly, ofAllowMultiSelect, ofEnableSizing]
    Left = 982
    Top = 48
  end
  object OpenDialogMERGE: TOpenDialog
    Filter = #1087#1077#1088#1076#1077#1090#1100' '#1085#1077#1083#1100#1079#1103' (*map*.txt)|*map*.txt|'#1084#1086#1078#1085#1086' '#1087#1077#1088#1076#1077#1090#1100' (*.*)|*.*'
    Left = 982
    Top = 80
  end
  object Timer: TTimer
    Enabled = False
    Interval = 350
    OnTimer = TimerTimer
    Left = 983
    Top = 15
  end
end
