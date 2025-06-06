import { HttpErrorResponse } from '@angular/common/http';
import { Component, Input, OnInit, OnDestroy } from '@angular/core';
import { FormBuilder, FormGroup, Validators } from '@angular/forms';
import { ToastrService } from 'ngx-toastr';
import { startWith, Subject, takeUntil } from 'rxjs';
import { LoadingService } from 'src/app/services/loading.service';
import { SystemService } from 'src/app/services/system.service';
import { eASICModel } from 'src/models/enum/eASICModel';
import { ActivatedRoute } from '@angular/router';

@Component({
  selector: 'app-edit',
  templateUrl: './edit.component.html',
  styleUrls: ['./edit.component.scss']
})
export class EditComponent implements OnInit, OnDestroy {

  public form!: FormGroup;

  public firmwareUpdateProgress: number | null = null;
  public websiteUpdateProgress: number | null = null;

  public savedChanges: boolean = false;
  public settingsUnlocked: boolean = false;
  public runningMode: number = 1;
  public eASICModel = eASICModel;
  public ASICModel!: eASICModel;
  public restrictedModels: eASICModel[] = Object.values(eASICModel)
    .filter((v): v is eASICModel => typeof v === 'string');

  @Input() uri = '';

  public ScreenSaverTime = [
    { name: 'Disabled', value: 0 },
    { name: '5 minute', value: 5 },
    { name: '10 minutes', value: 10 },
    { name: '15 minutes', value: 15 },
    { name: '30 minutes', value: 30 },
    { name: '60 minutes', value: 60 }
  ];

  public BM1397DropdownFrequency = [
    { name: '400', value: 400 },
    { name: '425 (default)', value: 425 },
    { name: '450', value: 450 },
    { name: '475', value: 475 },
    { name: '485', value: 485 },
    { name: '500', value: 500 },
    { name: '525', value: 525 },
    { name: '550', value: 550 },
    { name: '575', value: 575 },
    { name: '590', value: 590 },
    { name: '600', value: 600 },
    { name: '610', value: 610 },
    { name: '620', value: 620 },
    { name: '630', value: 630 },
    { name: '640', value: 640 },
    { name: '650', value: 650 },
  ];

  public BM1366DropdownFrequency = [
    { name: '400', value: 400 },
    { name: '425', value: 425 },
    { name: '450', value: 450 },
    { name: '475', value: 475 },
    { name: '485 (default)', value: 485 },
    { name: '500', value: 500 },
    { name: '525', value: 525 },
    { name: '550', value: 550 },
    { name: '575', value: 575 },
  ];

  public BM1368DropdownFrequency = [
    { name: '400', value: 400 },
    { name: '425', value: 425 },
    { name: '450', value: 450 },
    { name: '475', value: 475 },
    { name: '490 (default)', value: 490 },
    { name: '500', value: 500 },
    { name: '525', value: 525 },
    { name: '550', value: 550 },
    { name: '575', value: 575 },
  ];

  public BM1370DropdownFrequency = [
    { name: '400', value: 400 },
    { name: '490', value: 490 },
    { name: '525 (default)', value: 525 },
    { name: '550', value: 550 },
    { name: '575', value: 575 },
    //{ name: '596', value: 596 },
    { name: '600', value: 600 },
    { name: '625', value: 625 },
  ];

  public BM1370CoreVoltage = [
    { name: '1000', value: 1000 },
    { name: '1060', value: 1060 },
    { name: '1100', value: 1100 },
    { name: '1150 (default)', value: 1150 },
    { name: '1200', value: 1200 },
    { name: '1250', value: 1250 },
  ];

  public BM1397CoreVoltage = [
    { name: '1100', value: 1100 },
    { name: '1150', value: 1150 },
    { name: '1200', value: 1200 },
    { name: '1250', value: 1250 },
    { name: '1300', value: 1300 },
    { name: '1350', value: 1350 },
    { name: '1400', value: 1400 },
    { name: '1450', value: 1450 },
    { name: '1500', value: 1500 },
  ];
  public BM1366CoreVoltage = [
    { name: '1100', value: 1100 },
    { name: '1150', value: 1150 },
    { name: '1200 (default)', value: 1200 },
    { name: '1250', value: 1250 },
    { name: '1300', value: 1300 },
  ];
  public BM1368CoreVoltage = [
    { name: '1100', value: 1100 },
    { name: '1150', value: 1150 },
    { name: '1166 (default)', value: 1166 },
    { name: '1200', value: 1200 },
    { name: '1250', value: 1250 },
    { name: '1300', value: 1300 },
  ];

  private destroy$ = new Subject<void>();

  constructor(
    private fb: FormBuilder,
    private systemService: SystemService,
    private toastr: ToastrService,
    private loadingService: LoadingService,
    private route: ActivatedRoute,
  ) {
    // Check URL parameter for settings unlock
    this.route.queryParams.subscribe(params => {
      const urlOcParam = params['oc'] !== undefined;
      if (urlOcParam) {
        // If ?oc is in URL, enable overclock and save to NVS
        this.settingsUnlocked = true;
        this.saveOverclockSetting(1);
        console.log(
          '🎉 The ancient seals have been broken!\n' +
          '⚡ Unlimited power flows through your miner...\n' +
          '🔧 You can now set custom frequency and voltage values.\n' +
          '⚠️ Remember: with great power comes great responsibility!'
        );
      } else {
        // If ?oc is not in URL, check NVS setting (will be loaded in ngOnInit)
        console.log('🔒 Here be dragons! Advanced settings are locked for your protection. \n' +
          'Only the bravest miners dare to venture forth... \n' +
          'If you wish to unlock dangerous overclocking powers, add: %c?oc',
          'color: #ff4400; text-decoration: underline; cursor: pointer; font-weight: bold;',
          'to the current URL'
        );
      }
    });
  }

  private saveOverclockSetting(enabled: number) {
    this.systemService.updateSystem(this.uri, { overclockEnabled: enabled })
      .subscribe({
        next: () => {
          console.log(`Overclock setting saved: ${enabled === 1 ? 'enabled' : 'disabled'}`);
        },
        error: (err) => {
          console.error(`Failed to save overclock setting: ${err.message}`);
        }
      });
  }

  ngOnInit(): void {
    this.systemService.getInfo(this.uri)
      .pipe(
        this.loadingService.lockUIUntilComplete(),
        takeUntil(this.destroy$)
      )
      .subscribe(info => {
        this.ASICModel = info.ASICModel;
        this.runningMode = info.runningMode;

        // Check if overclock is enabled in NVS
        if (info.overclockEnabled === 1) {
          this.settingsUnlocked = true;
          console.log(
            '🎉 Overclock mode is enabled from NVS settings!\n' +
            '⚡ Custom frequency and voltage values are available.'
          );
        }

        this.form = this.fb.group({
          flipscreen: [info.flipscreen == 1],
          invertscreen: [info.invertscreen == 1],
          coreVoltage: [info.coreVoltage, [Validators.required]],
          frequency: [info.frequency, [Validators.required]],
          autofanspeed: [info.autofanspeed == 1, [Validators.required]],
          invertfanpolarity: [info.invertfanpolarity == 1, [Validators.required]],
          fanspeed: [info.fanspeed, [Validators.required]],
          beneathfanspeed:[info.beneathfanspeed,[Validators.required]],
          overheat_mode: [info.overheat_mode, [Validators.required]],
          screensavertime: [info.screensavertime, [Validators.required]],
          isChipRestart: [info.isChipRestart==1, [Validators.required]],
        });

        this.form.controls['autofanspeed'].valueChanges.pipe(
          startWith(this.form.controls['autofanspeed'].value),
          takeUntil(this.destroy$)
        ).subscribe(autofanspeed => {
          if (autofanspeed) {
            this.form.controls['fanspeed'].disable();
            this.form.controls['beneathfanspeed'].disable();
          } else {
            this.form.controls['fanspeed'].enable();
            this.form.controls['beneathfanspeed'].enable();
          }
        });
      });
  }

  ngOnDestroy(): void {
    this.destroy$.next();
    this.destroy$.complete();
  }

  public updateSystem() {

    const form = this.form.getRawValue();

    if (form.stratumPassword === '*****') {
      delete form.stratumPassword;
    }

    this.systemService.updateSystem(this.uri, form)
      .pipe(this.loadingService.lockUIUntilComplete())
      .subscribe({
        next: () => {
          const successMessage = this.uri ? `Saved settings for ${this.uri}` : 'Saved settings';
          this.toastr.success(successMessage, 'Success!');
          this.savedChanges = true;
        },
        error: (err: HttpErrorResponse) => {
          const errorMessage = this.uri ? `Could not save settings for ${this.uri}. ${err.message}` : `Could not save settings. ${err.message}`;
          this.toastr.error(errorMessage, 'Error');
          this.savedChanges = false;
        }
      });
  }

  showWifiPassword: boolean = false;
  toggleWifiPasswordVisibility() {
    this.showWifiPassword = !this.showWifiPassword;
  }

  changeRunningMode(mode: number){    
    this.systemService.updateSystem(this.uri, { runningMode: mode })
      .pipe(this.loadingService.lockUIUntilComplete())
      .subscribe({
        next: () => {
          this.runningMode = mode;
          console.log(`Runing mode change to: ${this.runningMode}`);
        },
        error: (err) => {
          console.error(`Failed to change running mode`);
        }
      });
  }

  disableOverheatMode() {
    this.form.patchValue({ overheat_mode: 0 });
    this.updateSystem();
  }

  toggleOverclockMode(enable: boolean) {
    this.settingsUnlocked = enable;
    this.saveOverclockSetting(enable ? 1 : 0);

    if (enable) {
      console.log(
        '🎉 Overclock mode enabled!\n' +
        '⚡ Custom frequency and voltage values are now available.'
      );
    } else {
      console.log('🔒 Overclock mode disabled. Using safe preset values only.');
    }
  }

  public restart() {
    this.systemService.restart(this.uri)
      .pipe(this.loadingService.lockUIUntilComplete())
      .subscribe({
        next: () => {
          const successMessage = this.uri ? `Bitaxe at ${this.uri} restarted` : 'Bitaxe restarted';
          this.toastr.success(successMessage, 'Success');
        },
        error: (err: HttpErrorResponse) => {
          const errorMessage = this.uri ? `Failed to restart device at ${this.uri}. ${err.message}` : `Failed to restart device. ${err.message}`;
          this.toastr.error(errorMessage, 'Error');
        }
      });
  }

  getScreenSaverTime() {
    // Get base screen saver options
    let options = [...this.ScreenSaverTime];

    // Get current screen saver value from form
    const currentScreenSaver = this.form?.get('screensaver')?.value;

    // If current screen saver exists and isn't in the options
    if (currentScreenSaver && !options.some(opt => opt.value === currentScreenSaver)) {
        options.push({
            name: `${currentScreenSaver} (Custom)`,
            value: currentScreenSaver
        });
        // Sort options by value
        options.sort((a, b) => a.value - b.value);
    }

    return options;
  }

  getDropdownFrequency() {
    // Get base frequency options based on ASIC model
    let options = [];
    switch(this.ASICModel) {
        case this.eASICModel.BM1366: options = [...this.BM1366DropdownFrequency]; break;
        case this.eASICModel.BM1368: options = [...this.BM1368DropdownFrequency]; break;
        case this.eASICModel.BM1370: options = [...this.BM1370DropdownFrequency]; break;
        case this.eASICModel.BM1397: options = [...this.BM1397DropdownFrequency]; break;
        default: return [];
    }

    // Get current frequency value from form
    const currentFreq = this.form?.get('frequency')?.value;

    // If current frequency exists and isn't in the options
    if (currentFreq && !options.some(opt => opt.value === currentFreq)) {
        options.push({
            name: `${currentFreq} (Custom)`,
            value: currentFreq
        });
        // Sort options by frequency value
        options.sort((a, b) => a.value - b.value);
    }

    return options;
  }

  getCoreVoltage() {
    // Get base voltage options based on ASIC model
    let options = [];
    switch(this.ASICModel) {
        case this.eASICModel.BM1366: options = [...this.BM1366CoreVoltage]; break;
        case this.eASICModel.BM1368: options = [...this.BM1368CoreVoltage]; break;
        case this.eASICModel.BM1370: options = [...this.BM1370CoreVoltage]; break;
        case this.eASICModel.BM1397: options = [...this.BM1397CoreVoltage]; break;
        default: return [];
    }

    // Get current voltage value from form
    const currentVoltage = this.form?.get('coreVoltage')?.value;

    // If current voltage exists and isn't in the options
    if (currentVoltage && !options.some(opt => opt.value === currentVoltage)) {
        options.push({
            name: `${currentVoltage} (Custom)`,
            value: currentVoltage
        });
        // Sort options by voltage value
        options.sort((a, b) => a.value - b.value);
    }

    return options;
  }

}
