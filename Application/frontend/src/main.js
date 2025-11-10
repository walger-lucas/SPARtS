import { createApp } from 'vue'
import App from './App.vue'
import router from './router'
import PrimeVue from 'primevue/config';
import Aura from '@primeuix/themes/aura';
import "primeflex/primeflex.css"
import 'primeicons/primeicons.css';
import { ToastService } from 'primevue';
import { definePreset } from '@primeuix/themes';


const app = createApp(App)

const customTheme = definePreset(Aura,{
    semantic:{
        primary:{
            50: '{red.50}',
            100: '{red.100}',
            200: '{red.200}',
            300: '{red.300}',
            400: '{red.400}',
            500: '{red.500}',
            600: '{red.600}',
            700: '{red.700}',
            800: '{red.800}',
            900: '{red.900}',
            950: '{red.950}'
        }
    },
    components:{
        panel:{
            border:{
                color: '#000000'
            } 
        }
    }
})

app.use(router)
app.use(ToastService)
app.use(PrimeVue, {
    theme: {
        preset: customTheme,
        options: {
            darkModeSelector: false,
        }
    }
});
app.mount('#app')
